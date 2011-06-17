# 
#  Copyright 2005  Red Hat, Inc.
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Red Hat not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Red Hat makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

bin_PROGRAMS = xinit
bin_SCRIPTS = startx

AM_CFLAGS = $(CWARNFLAGS) $(XINIT_CFLAGS)
AM_CPPFLAGS = -DXINITDIR=\"$(XINITDIR)\" -DBINDIR=\"$(bindir)\"
xinit_LDADD = $(XINIT_LIBS)

xinit_SOURCES =	\
        xinit.c

appmandir = $(APP_MAN_DIR)
appman_PRE = startx.man  xinit.man
appman_DATA = $(appman_PRE:man=$(APP_MAN_SUFFIX))

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
MAN_SUBSTS+=	-e 's|__XSERVERNAME__|$(XSERVERNAME)|g' \
		-e 's|__XCONFIGFILE__|$(XCONFIGFILE)|g' \
		-e 's|__XCONFIGFILEMAN__|$(XCONFIGFILEMAN)|g' \
		-e 's|__xinitdir__|$(XINITDIR)|g' \
		-e 's|__bindir__|$(bindir)|g' \
		-e 's|__libdir__|$(libdir)|g' \
		-e 's|__SCOMAN__|$(SCOMAN)|g'

include $(top_srcdir)/cpprules.in

SUFFIXES += .$(APP_MAN_SUFFIX) .man
.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

xinitrcdir = $(XINITDIR)

PROGCPPDEFS = \
	-DXRDB=@XRDB@ \
	-DXMODMAP=@XMODMAP@ \
	-DTWM=@TWM@ \
	-DXCLOCK=@XCLOCK@ \
	-DXTERM=@XTERM@ \
	-DXSERVER=@XSERVER@ \
	-DXAUTH=@XAUTH@ \
	-DXINIT=@XINIT@

CPP_FILES_FLAGS = \
	-DXINITDIR=$(XINITDIR) $(PROGCPPDEFS) -DLIBDIR=$(libdir) \
	-DSHELL_CMD=$(SHELL_CMD) $(STARTX_COOKIE_FLAGS) \
	-D__libexecdir__="$(libexecdir)" \
	-D__bindir__="$(bindir)" \
	-DBUNDLE_ID_PREFIX="$(bundleidprefix)"

if LAUNCHD
SUBDIRS = launchd
endif

DIST_SUBDIRS = launchd

xinitrc_DATA = xinitrc

MAINTAINERCLEANFILES = ChangeLog INSTALL
CLEANFILES = xinitrc startx $(appman_DATA) $(launchagents_DATA)

EXTRA_DIST = xinitrc.cpp startx.cpp $(appman_PRE)	\
		autogen.sh

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
