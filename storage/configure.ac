#
# Options:
# --enable-libuv=no   	enable libuv UDP/TCP. Default UDP only
# --enable-debug=no   	enable debugging output
# --enable-json=no      enable JSON backend
# --enable-gen=no       enable gen backend
# --enable-sqlite=no    enable SQLite backend
# --enable-ipv6=no    	enable IPv6(reserved)
# --enable-tests=no     enable tests

# --enable-http=no		enable HTTP service. libmicrohttpd dependency required.
# --enable-qrcode=no	enable HTTP QRCode URN service. libmicrohttpd dependency required.

# Example:
# ./configure --enable-gen=yes --enable-libuv=yes
#
AC_PREREQ([2.63])
AC_INIT([lorawan-storage], [1.0], [andrei.i.ivanov@gmail.com])
AM_INIT_AUTOMAKE([1.11 foreign no-define subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([cli-main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

LT_INIT

# determine target OS
build_linux=no
build_windows=no
build_mac=no
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_mac=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac
# Pass the conditionals to automake
AM_CONDITIONAL([OS_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([OS_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OS_MAC], [test "$build_mac" = "yes"])

AC_ARG_ENABLE([tests],
[  --enable-tests    Turn on tests],
[case "${enableval}" in
  yes) tests=true ;;
  no)  tests=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-tests]) ;;
esac],[tests=false])
AM_CONDITIONAL([ENABLE_TESTS], [test x$tests = xtrue])

AC_ARG_ENABLE([libuv],
[  --enable-libuv    Turn on libuv],
[case "${enableval}" in
  yes) libuv=true ;;
  no)  libuv=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-libuv]) ;;
esac],[libuv=false])
AM_CONDITIONAL([ENABLE_LIBUV], [test x$libuv = xtrue])

AC_ARG_ENABLE([json],
[  --enable-json    Turn on JSON file backend],
[case "${enableval}" in
  yes) json=true ;;
  no)  json=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-json]) ;;
esac],[json=false])
AM_CONDITIONAL([ENABLE_JSON], [test x$json = xtrue])

AC_ARG_ENABLE([http],
[  --enable-json    Turn on HTTP listener],
[case "${enableval}" in
  yes) http=true ;;
  no)  http=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-http]) ;;
esac],[http=false])
AM_CONDITIONAL([ENABLE_HTTP], [test x$http = xtrue])

AC_ARG_ENABLE([gen],
[  --enable-gen    Turn on gen backend],
[case "${enableval}" in
  yes) gen=true ;;
  no)  gen=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-gen]) ;;
esac],[gen=false])
AM_CONDITIONAL([ENABLE_GEN], [test x$gen = xtrue])

AC_ARG_ENABLE([sqlite],
[  --enable-sqlite    Turn on SQLite backend],
[case "${enableval}" in
  yes) sqlite=true ;;
  no)  sqlite=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-sqlite]) ;;
esac],[sqlite=false])
AM_CONDITIONAL([ENABLE_SQLITE], [test x$sqlite = xtrue])

AC_ARG_ENABLE([ipv6],
[  --enable-ipv6    Turn on IPv6 support],
[case "${enableval}" in
  yes) ipv6=true ;;
  no)  ipv6=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-ipv6]) ;;
esac],[ipv6=false])
AM_CONDITIONAL([ENABLE_IPV6], [test x$ipv6 = xtrue])

AC_ARG_ENABLE([debug],
[  --enable-debug    Turn on debug outout],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([ENABLE_DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([http],
[  --enable-http    enable HTTP service],
[case "${enableval}" in
  yes) http=true ;;
  no)  http=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-http]) ;;
esac],[http=false])
AM_CONDITIONAL([ENABLE_HTTP], [test x$http = xtrue])

AC_ARG_ENABLE([qrcode],
[  --enable-qrcode    enable HTTP QRCode URN service],
[case "${enableval}" in
  yes) qrcode=true ;;
  no)  qrcode=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-qrcode]) ;;
esac],[qrcode=false])
AM_CONDITIONAL([ENABLE_QRCODE], [test x$qrcode = xtrue])

build_mac=no
# Detect the target system
case "${host_os}" in
  darwin*)
    build_mac=yes
    ;;
esac
AM_CONDITIONAL([OS_MACH], [test "$build_mac" = "yes"])

# MacOS: install openssl into /usr/local/lib/opt/openssl@1.1
# brew install openssl@1.1
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# Checks for header files. Add /usr/local/include for OS X.
CFLAGS="$CFLAGS -I/usr/local/include"

if test "$sqlite" = "true"; then
AC_CHECK_HEADERS(sqlite3.h)
AC_CHECK_LIB(sqlite3,sqlite3_close)
fi

if test "$http" = "true"; then
AC_CHECK_HEADERS(microhttpd.h)
AC_CHECK_LIB(MHD_create_response_from_buffer)
fi

AC_OUTPUT
