XCOMM!SHELL_CMD
XCOMM $Xorg: xinitrc.cpp,v 1.3 2000/08/17 19:54:30 cpqbld Exp $

userresources=$HOME/.Xresources
usermodmap=$HOME/.Xmodmap
sysresources=XINITDIR/.Xresources
sysmodmap=XINITDIR/.Xmodmap

XCOMM merge in defaults and keymaps

if [ -f $sysresources ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge $sysresources
    else
        XRDB -nocpp -merge $sysresources
    fi
#else
    XRDB -merge $sysresources
#endif
fi

if [ -f $sysmodmap ]; then
    XMODMAP $sysmodmap
fi

if [ -f $userresources ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge $userresources
    else
        XRDB -nocpp -merge $userresources
    fi
#else
    XRDB -merge $userresources
#endif
fi

if [ -f $usermodmap ]; then
    XMODMAP $usermodmap
fi

XCOMM start some nice programs

#if defined(__SCO__) || defined(__UNIXWARE__)
if [ -r /etc/default/xdesktops ]; then
  . /etc/default/xdesktops
fi

if [ -r $HOME/.x11rc ]; then
  . $HOME/.x11rc
else
  if [ -r /etc/default/X11 ]; then
  . /etc/default/X11
  fi
fi

#if defined(__SCO__)
if [ -n "$XSESSION" ]; then
  case "$XSESSION" in
    [Yy][Ee][Ss])
      [ -x /usr/bin/X11/scosession ] && exec /usr/bin/X11/scosession
      ;;
  esac
fi

if [ -n "$XDESKTOP" ]; then
  exec `eval echo $"$XDESKTOP"`
else
  if [ -x /usr/bin/X11/pmwm -a -x /usr/bin/X11/scoterm ]; then
    /usr/bin/X11/scoterm 2> /dev/null &
    exec /usr/bin/X11/pmwm    2> /dev/null
  fi
fi
#elif defined(__UNIXWARE__)
if [ -n "$XDESKTOP" ]; then
  exec `eval echo $"$XDESKTOP"`
else
  if [ -x /usr/X/bin/pmwm ]; then
    exec /usr/X/bin/pmwm    2> /dev/null
  fi
fi
#endif

XCOMM This is the fallback case if nothing else is executed above
#endif /* !defined(__SCO__)  && !defined(__UNIXWARE__) */

#ifdef __APPLE__

[ -x /usr/X11/bin/font_cache.sh ] && /usr/X11/bin/font_cache.sh

if [ -x /usr/X11/bin/xset ] ; then
	fontpath="/usr/X11/lib/X11/fonts/misc/,/usr/X11/lib/X11/fonts/TTF/,/usr/X11/lib/X11/fonts/OTF,/usr/X11/lib/X11/fonts/Type1/,/usr/X11/lib/X11/fonts/75dpi:unscaled/,/usr/X11/lib/X11/fonts/100dpi/:unscaled,/usr/X11/lib/X11/fonts/75dpi:unscaled/,/usr/X11/lib/X11/fonts/100dpi/:unscaled"

	[ -e "$HOME"/.fonts/fonts.dir ] && fontpath="$fontpath,$HOME/.fonts"
	[ -e "$HOME"/Library/Fonts/fonts.dir ] && fontpath="$fontpath,$HOME/Library/Fonts"
	[ -e /Library/Fonts/fonts.dir ] && fontpath="$fontpath,/Library/Fonts"
	[ -e /System/Library/Fonts/fonts.dir ] && fontpath="$fontpath,/System/Library/Fonts"

	/usr/X11/bin/xset fp= "$fontpath"
	unset fontpath
fi

[ -x /usr/bin/quartz-wm ] && exec /usr/bin/quartz-wm

#endif

TWM &
XCLOCK -geometry 50x50-1+1 &
XTERM -geometry 80x50+494+51 &
XTERM -geometry 80x20+494-0 &
exec XTERM -geometry 80x66+0+0 -name login
