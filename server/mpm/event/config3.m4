dnl ## XXX - Need a more thorough check of the proper flags to use

if ap_mpm_is_enabled "event"; then
    AC_CHECK_FUNCS(pthread_kill)
    APACHE_FAST_OUTPUT(server/mpm/event/Makefile)
fi
