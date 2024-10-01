#
# ./configure [option]
# Options:
# --enable-sqlite=no  	    enable sqlite3 backend
# --enable-gw-dev-usb=no    build RAK2287 USB gateway backend
# --enable-libuv=no   	    enable libuv UDP/TCP. Default UDP only
# --enable-debug=no   	    enable debugging output
# --enable-gen=no          	enable key generator (default in memory storage)
# --enable-mqtt=no          build with MQTT bridge
# --enable-json=no 	        enable JSON file backend

#
AC_PREREQ([2.63])
AC_INIT([tlns], [0.1], [andrei.i.ivanov@gmail.com])
AM_INIT_AUTOMAKE([1.11 foreign no-define subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([cli-main-check.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
LT_INIT

AC_ARG_ENABLE([sqlite],
[  --enable-sqlite    enable sqlite3 backend],
[case "${enableval}" in
  yes) sqlite=true ;;
  no)  sqlite=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-sqlite]) ;;
esac],[sqlite=false])
AM_CONDITIONAL([ENABLE_SQLITE], [test x$sqlite = xtrue])

AC_ARG_ENABLE([gw-dev-usb],
[  --enable-gw-dev-usb    build RAK2287 USB gateway backend],
[case "${enableval}" in
  yes) gw_dev_usb=true ;;
  no)  gw_dev_usb=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-gw-dev-usb]) ;;
esac],[gw_dev_usb=false])
AM_CONDITIONAL([ENABLE_GW_DEV_USB], [test x$gw_dev_usb = xtrue])

AC_ARG_ENABLE([libuv],
[  --enable-libuv    enable libuv UDP/TCP. Default UDP only],
[case "${enableval}" in
  yes) libuv=true ;;
  no)  libuv=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-libuv]) ;;
esac],[libuv=false])
AM_CONDITIONAL([ENABLE_LIBUV], [test x$libuv = xtrue])

AC_ARG_ENABLE([debug],
[  --enable-debug    enable debugging output],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([ENABLE_DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([gen],
[  --enable-gen    enable key generator (default in memory storage)],
[case "${enableval}" in
  yes) gen=true ;;
  no)  gen=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-gen]) ;;
esac],[gen=false])
AM_CONDITIONAL([ENABLE_GEN], [test x$gen = xtrue])

AC_ARG_ENABLE([mqtt],
[  --enable-mqtt    build with MQTT bridge)],
[case "${enableval}" in
  yes) mqtt=true ;;
  no)  mqtt=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-mqtt]) ;;
esac],[gen=false])
AM_CONDITIONAL([ENABLE_MQTT], [test x$mqtt = xtrue])

AC_ARG_ENABLE([json],
[  --enable-json    enable JSON file backend],
[case "${enableval}" in
  yes) json=true ;;
  no)  json=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-json]) ;;
esac],[json=false])
AM_CONDITIONAL([ENABLE_JSON], [test x$json = xtrue])

if test "$sqlite" = "true"; then
AC_CHECK_HEADERS(sqlite3.h)
AC_CHECK_LIB(sqlite3,sqlite3_close)
fi

if test "$libuv" = "true"; then
AC_CHECK_HEADERS(uv.h)
AC_CHECK_LIB(uv_write,uv_udp_init,uv_run)
fi

AC_MSG_RESULT([+--------------------------------------])
if test "$sqlite" = "true"; then
AC_MSG_RESULT([sqlite3 backend:                       yes])
else
AC_MSG_RESULT([sqlite3 backend (--enable-sqlite):     no])
fi
if test "$gw_dev_usb" = "true"; then
AC_MSG_RESULT([build RAK2287 USB gateway backend:     yes])
else
AC_MSG_RESULT([RAK2287 gateway (--enable-gw-dev-usb): no])
fi
if test "$libuv" = "true"; then
AC_MSG_RESULT([libuv UDP/TCP:                         yes])
else
AC_MSG_RESULT([libuv UDP/TCP (--enable-libuv):        no])
fi
if test "$debug" = "true"; then
AC_MSG_RESULT([debugging output:                      yes])
else
AC_MSG_RESULT([debugging output (--enable-debug):     no])
fi
if test "$gen" = "true"; then
AC_MSG_RESULT([key generator:                         yes])
else
AC_MSG_RESULT([key generator (--enable-gen):          no])
fi
if test "$json" = "true"; then
AC_MSG_RESULT([JSON file backend:                     yes])
else
AC_MSG_RESULT([JSON file backend (--enable-json):     no])
fi
if test "$mqtt" = "true"; then
AC_MSG_RESULT([build with MQTT (--enable-mqtt):       yes])
else
AC_MSG_RESULT([build with MQTT (--enable-mqtt):       no])
fi
AC_MSG_RESULT([+--------------------------------------])

AC_OUTPUT
