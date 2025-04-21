# filepath: /home/nico/GITHUB_REPOS/tcp_congestion_linux_cmp/configure.ac
AC_INIT([TCP Congestion Policies Comparison], [1.0], [your.email@example.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h sys/socket.h unistd.h])

# Check for BCC headers
AC_MSG_CHECKING([for minimal BCC headers])
BCC_MINIMAL_DIR="$srcdir/src/bcc_minimal/include"
if test -f "$BCC_MINIMAL_DIR/bcc/bcc_common.h"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([HAVE_BCC], [1], [Define if BCC headers are available])
    AC_DEFINE([ENABLE_EBPF_METRICS], [1], [Define to enable eBPF metrics])
else
    AC_MSG_RESULT([no])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
