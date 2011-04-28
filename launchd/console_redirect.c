/* Copyright (c) 2011 Apple Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 * HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without
 * prior written authorization.
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/event.h>
#include <asl.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#define BUF_SIZE 1024

typedef struct {
    /* Initialized values */
    int fd;
    int level;
    aslclient aslc;
    aslmsg aslm;

    /* Buffer for reading */
    char buf[BUF_SIZE];
    char *w;
    int closed;
} asl_redirect;

/* Redirect stdout/stderr to asl */
static void *redirect_thread(void *ctx) {
    asl_redirect *fds = ctx;
    char *p, *s;
    ssize_t nbytes;
    struct kevent ev[2];
    int kq, n;

    /* Setup our kqueue */
    kq = kqueue();
    EV_SET(&ev[0], fds[0].fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    EV_SET(&ev[1], fds[1].fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    kevent(kq, ev, 2, NULL, 0, NULL);

    /* Set our buffers to empty */
    fds[0].w = fds[0].buf;
    fds[1].w = fds[1].buf;

    /* Start off open */
    fds[0].closed = fds[1].closed = 0;

    while(!(fds[0].closed && fds[1].closed)) {
        n = kevent(kq, NULL, 0, ev, 1, NULL);
        if(n < 0) {
            asl_log(fds[1].aslc, fds[1].aslm, ASL_LEVEL_ERR, "read failure: %s", strerror(errno));
            break;
        }

        if(n == 1 && ev->filter == EVFILT_READ) {
            int fd = ev->ident;
            asl_redirect *aslr;

            if(fd == fds[0].fd && !fds[0].closed) {
                aslr = &fds[0];
            } else if(fd == fds[1].fd && !fds[1].closed) {
                aslr = &fds[1];
            } else {
                asl_log(fds[1].aslc, fds[1].aslm, ASL_LEVEL_ERR, "unexpected file descriptor: %d", fd);
                break;
            }

            if(ev->flags & EV_EOF) {
                EV_SET(&ev[1], aslr->fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
                kevent(kq, &ev[1], 1, NULL, 0, NULL);
                aslr->closed = 1;
            }

            nbytes = read(aslr->fd, aslr->w, BUF_SIZE - (aslr->w - aslr->buf) - 1);
            if(nbytes > 0) {
                nbytes += (aslr->w - aslr->buf);
                aslr->buf[nbytes] = '\0';

                /* One line at a time */
                for(p=aslr->buf; *p && (p - aslr->buf) < nbytes; p = s + 1) {
                    // Find null or \n
                    for(s=p; *s && *s != '\n'; s++);
                    if(*s == '\n') {
                        *s='\0';
                        asl_log(aslr->aslc, aslr->aslm, aslr->level, "%s", p);
                    } else if(aslr->buf != p) {
                        memmove(aslr->buf, p, BUF_SIZE);
                        aslr->w = aslr->buf + (s - p);
                        break;
                    } else if(nbytes == BUF_SIZE - 1) {
                        asl_log(aslr->aslc, aslr->aslm, aslr->level, "%s", p);
                        aslr->w = aslr->buf;
                        break;
                    }
                }
            }

            if(aslr->closed) {
                close(aslr->fd);
                if(aslr->w > aslr->buf) {
                    *aslr->w = '\0';
                    asl_log(aslr->aslc, aslr->aslm, aslr->level, "%s", aslr->buf);
                }
            }
        }
    }

    return NULL;
}

static pthread_t redirect_pthread;
static void redirect_atexit(void) {
    /* stdout is linebuffered, so flush the buffer */
    fflush(stdout);

    /* close our pipes, causing the redirect thread to terminate */
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    pthread_join(redirect_pthread, NULL);
}

int console_redirect(aslclient aslc, aslmsg aslm, int stdout_level, int stderr_level) {
    int err;
    int outpair[2];
    int errpair[2];
    static asl_redirect readpair[2];

    /* Create pipes */
    if(pipe(outpair) == -1) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "pipe() failed: %s", strerror(errno));
        return errno;
    }

    if(pipe(errpair) == -1) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "pipe() failed: %s", strerror(errno));
        close(outpair[0]);
        close(outpair[1]);
        return errno;
    }

    /* Close the read fd but not the write fd on exec */
    fcntl(outpair[0], F_SETFD, FD_CLOEXEC);
    fcntl(errpair[0], F_SETFD, FD_CLOEXEC);

    /* Setup the context to handoff to the read thread */
    readpair[0].fd = outpair[0];
    readpair[0].level = stdout_level;
    readpair[0].aslc = aslc;
    readpair[0].aslm = aslm;

    readpair[1].fd = errpair[0];
    readpair[1].level = stderr_level;
    readpair[1].aslc = aslc;
    readpair[1].aslm = aslm;

    /* Handoff to the read thread */
    if((err = pthread_create(&redirect_pthread, NULL, redirect_thread, readpair)) != 0) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "pthread_create() failed: %s", strerror(err));
        close(outpair[0]);
        close(outpair[1]);
        close(errpair[0]);
        close(errpair[1]);
        return err;
    }

    /* Replace our stdout fd.  force stdout to be line buffered since it defaults to buffered when not a tty */
    if(dup2(outpair[1], STDOUT_FILENO) == -1) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "dup2(stdout) failed: %s", strerror(errno));
    } else if(setlinebuf(stdout) != 0) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "setlinebuf(stdout) failed, log redirection may be delayed.");
    }

    /* Replace our stderr fd.  stderr is always unbuffered */
    if(dup2(errpair[1], STDERR_FILENO) == -1) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "dup2(stderr) failed: %s", strerror(errno));
    }

    /* Close the duplicate fds since they've been reassigned */
    close(outpair[1]);
    close(errpair[1]);

    /* Register an atexit handler to wait for the logs to flush */
    if(atexit(redirect_atexit) != 0) {
        asl_log(aslc, aslm, ASL_LEVEL_ERR, "atexit(redirect_atexit) failed: %s", strerror(errno));
    }

    return 0;
}

#ifdef DEBUG_CONSOLE_REDIRECT
int main(int argc, char **argv) {
    console_redirect(NULL, NULL, ASL_LEVEL_NOTICE, ASL_LEVEL_ERR);

    fprintf(stderr, "TEST ERR1\n");
    fprintf(stdout, "TEST OUT1\n");
    fprintf(stderr, "TEST ERR2\n");
    fprintf(stdout, "TEST OUT2\n");
    system("/bin/echo SYST OUT");
    system("/bin/echo SYST ERR >&2");
    fprintf(stdout, "TEST OUT3\n");
    fprintf(stdout, "TEST OUT4");
    fprintf(stderr, "TEST ERR3\n");
    fprintf(stderr, "TEST ERR4");

    exit(0);
}
#endif
