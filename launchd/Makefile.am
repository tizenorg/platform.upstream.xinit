if APPLE
libexec_PROGRAMS = x11-exec
x11_exec_LDFLAGS = -framework CoreServices
endif

launchagents_PRE = org.x.X11.plist.pre
launchagents_DATA = $(launchagents_PRE:plist.pre=plist)

CPP_FILES_FLAGS = \
	-D__libexecdir__="$(libexecdir)" \
	-D__bindir__="$(bindir)"

CLEANFILES = $(launchagents_DATA)

include $(top_srcdir)/cpprules.in
