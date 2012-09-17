exp_exec_prefix = /usr/local2/apache2
rel_exec_prefix =
exp_bindir = /usr/local2/apache2/bin
rel_bindir = bin
exp_sbindir = /usr/local2/apache2/bin
rel_sbindir = bin
exp_libdir = /usr/local2/apache2/lib
rel_libdir = lib
exp_libexecdir = /usr/local2/apache2/modules
rel_libexecdir = modules
exp_mandir = /usr/local2/apache2/man
rel_mandir = man
exp_sysconfdir = /usr/local2/apache2/conf
rel_sysconfdir = conf
exp_datadir = /usr/local2/apache2
rel_datadir =
exp_installbuilddir = /usr/local2/apache2/build
rel_installbuilddir = build
exp_errordir = /usr/local2/apache2/error
rel_errordir = error
exp_iconsdir = /usr/local2/apache2/icons
rel_iconsdir = icons
exp_htdocsdir = /usr/local2/apache2/htdocs
rel_htdocsdir = htdocs
exp_manualdir = /usr/local2/apache2/manual
rel_manualdir = manual
exp_cgidir = /usr/local2/apache2/cgi-bin
rel_cgidir = cgi-bin
exp_includedir = /usr/local2/apache2/include
rel_includedir = include
exp_localstatedir = /usr/local2/apache2
rel_localstatedir =
exp_runtimedir = /usr/local2/apache2/logs
rel_runtimedir = logs
exp_logfiledir = /usr/local2/apache2/logs
rel_logfiledir = logs
exp_proxycachedir = /usr/local2/apache2/proxy
rel_proxycachedir = proxy
EGREP = /usr/bin/grep -E
PCRE_LIBS = -L/usr/local/lib -lpcre
SHLTCFLAGS = -prefer-pic
LTCFLAGS = -prefer-non-pic -static
MKINSTALLDIRS = $(abs_srcdir)/build/mkdir.sh
INSTALL = $(LIBTOOL) --mode=install $(abs_srcdir)/build/install.sh -c
MATH_LIBS =
CRYPT_LIBS =
DTRACE = true
PICFLAGS =
PILDFLAGS =
MOD_AUTHN_FILE_LDADD =
MOD_AUTHN_DBM_LDADD =
MOD_AUTHN_ANON_LDADD =
MOD_AUTHN_DBD_LDADD =
MOD_AUTHN_SOCACHE_LDADD =
MOD_AUTHN_CORE_LDADD =
MOD_AUTHZ_HOST_LDADD =
MOD_AUTHZ_GROUPFILE_LDADD =
MOD_AUTHZ_USER_LDADD =
MOD_AUTHZ_DBM_LDADD =
MOD_AUTHZ_OWNER_LDADD =
MOD_AUTHZ_DBD_LDADD =
MOD_AUTHZ_CORE_LDADD =
MOD_ACCESS_COMPAT_LDADD =
MOD_AUTH_BASIC_LDADD =
MOD_AUTH_FORM_LDADD =
MOD_AUTH_DIGEST_LDADD =
MOD_ALLOWMETHODS_LDADD =
MOD_APREQ_LDADD =
MOD_FILE_CACHE_LDADD =
MOD_CACHE_LDADD =
MOD_CACHE_DISK_LDADD =
MOD_SOCACHE_SHMCB_LDADD =
MOD_SOCACHE_DBM_LDADD =
MOD_SOCACHE_MEMCACHE_LDADD =
INSTALL_DSO = yes
MOD_SO_LDADD =
MOD_DBD_LDADD =
MOD_DUMPIO_LDADD =
MOD_FIREHOSE_LDADD =
MOD_BUFFER_LDADD =
MOD_RATELIMIT_LDADD =
MOD_REQTIMEOUT_LDADD =
MOD_EXT_FILTER_LDADD =
MOD_REQUEST_LDADD =
MOD_INCLUDE_LDADD =
MOD_FILTER_LDADD =
MOD_SUBSTITUTE_LDADD =
MOD_SED_LDADD =
MOD_DEFLATE_LDADD = -L/usr/local/lib -lz
MOD_HTTP_LDADD =
MOD_MIME_LDADD =
MOD_LOG_CONFIG_LDADD =
MOD_LOG_DEBUG_LDADD =
MOD_LOGIO_LDADD =
MOD_ENV_LDADD =
MOD_EXPIRES_LDADD =
MOD_HEADERS_LDADD =
MOD_UNIQUE_ID_LDADD =
MOD_SETENVIF_LDADD =
MOD_VERSION_LDADD =
MOD_REMOTEIP_LDADD =
MOD_PROXY_LDADD =
MOD_PROXY_CONNECT_LDADD =
MOD_PROXY_FTP_LDADD =
MOD_PROXY_HTTP_LDADD =
MOD_PROXY_FCGI_LDADD =
MOD_PROXY_SCGI_LDADD =
MOD_PROXY_AJP_LDADD =
MOD_PROXY_BALANCER_LDADD =
MOD_PROXY_EXPRESS_LDADD =
MOD_SESSION_LDADD =
MOD_SESSION_COOKIE_LDADD =
MOD_SESSION_DBD_LDADD =
MOD_SLOTMEM_SHM_LDADD =
ab_CFLAGS = -I/opt/local/include
ab_LDFLAGS = -L/opt/local/lib -lssl -lcrypto -lpthread
MOD_SSL_LDADD =
htpasswd_LTFLAGS =
htdigest_LTFLAGS =
rotatelogs_LTFLAGS =
logresolve_LTFLAGS =
htdbm_LTFLAGS =
ab_LTFLAGS =
checkgid_LTFLAGS =
htcacheclean_LTFLAGS =
httxt2dbm_LTFLAGS =
fcgistarter_LTFLAGS =
firehose_LTFLAGS =
NONPORTABLE_SUPPORT = checkgid fcgistarter
APACHECTL_ULIMIT =
MOD_LBMETHOD_BYREQUESTS_LDADD =
MOD_LBMETHOD_BYTRAFFIC_LDADD =
MOD_LBMETHOD_BYBUSYNESS_LDADD =
MOD_LBMETHOD_HEARTBEAT_LDADD =
MPM_SUBDIRS = event
MPM_LIB = server/mpm/event/libevent.la
SERF_LIBS =
MOD_MPM_EVENT_LDADD =
MOD_UNIXD_LDADD =
MOD_DAV_LDADD =
MOD_STATUS_LDADD =
MOD_AUTOINDEX_LDADD =
MOD_INFO_LDADD =
MOD_CGID_LDADD =
MOD_DAV_FS_LDADD =
MOD_VHOST_ALIAS_LDADD =
MOD_NEGOTIATION_LDADD =
MOD_DIR_LDADD =
MOD_ACTIONS_LDADD =
MOD_SPELING_LDADD =
MOD_USERDIR_LDADD =
MOD_ALIAS_LDADD =
MOD_REWRITE_LDADD =
progname = httpd
OS = unix
OS_DIR = unix
BUILTIN_LIBS = modules/aaa/libmod_authn_file.la modules/aaa/libmod_authn_dbm.la modules/aaa/libmod_authn_anon.la modules/aaa/libmod_authn_dbd.la modules/aaa/libmod_authn_socache.la modules/aaa/libmod_authn_core.la modules/aaa/libmod_authz_host.la modules/aaa/libmod_authz_groupfile.la modules/aaa/libmod_authz_user.la modules/aaa/libmod_authz_dbm.la modules/aaa/libmod_authz_owner.la modules/aaa/libmod_authz_dbd.la modules/aaa/libmod_authz_core.la modules/aaa/libmod_access_compat.la modules/aaa/libmod_auth_basic.la modules/aaa/libmod_auth_form.la modules/aaa/libmod_auth_digest.la modules/aaa/libmod_allowmethods.la modules/apreq/libmod_apreq.la modules/cache/libmod_file_cache.la modules/cache/libmod_cache.la modules/cache/libmod_cache_disk.la modules/cache/libmod_socache_shmcb.la modules/cache/libmod_socache_dbm.la modules/cache/libmod_socache_memcache.la modules/core/libmod_so.la modules/database/libmod_dbd.la modules/debugging/libmod_dumpio.la modules/debugging/libmod_firehose.la modules/filters/libmod_buffer.la modules/filters/libmod_ratelimit.la modules/filters/libmod_reqtimeout.la modules/filters/libmod_ext_filter.la modules/filters/libmod_request.la modules/filters/libmod_include.la modules/filters/libmod_filter.la modules/filters/libmod_substitute.la modules/filters/libmod_sed.la modules/filters/libmod_deflate.la modules/http/libmod_http.la modules/http/libmod_mime.la modules/loggers/libmod_log_config.la modules/loggers/libmod_log_debug.la modules/loggers/libmod_logio.la modules/metadata/libmod_env.la modules/metadata/libmod_expires.la modules/metadata/libmod_headers.la modules/metadata/libmod_unique_id.la modules/metadata/libmod_setenvif.la modules/metadata/libmod_version.la modules/metadata/libmod_remoteip.la modules/proxy/libmod_proxy.la modules/proxy/libmod_proxy_connect.la modules/proxy/libmod_proxy_ftp.la modules/proxy/libmod_proxy_http.la modules/proxy/libmod_proxy_fcgi.la modules/proxy/libmod_proxy_scgi.la modules/proxy/libmod_proxy_ajp.la modules/proxy/libmod_proxy_balancer.la modules/proxy/libmod_proxy_express.la modules/session/libmod_session.la modules/session/libmod_session_cookie.la modules/session/libmod_session_dbd.la modules/slotmem/libmod_slotmem_shm.la modules/ssl/libmod_ssl.la modules/proxy/balancers/libmod_lbmethod_byrequests.la modules/proxy/balancers/libmod_lbmethod_bytraffic.la modules/proxy/balancers/libmod_lbmethod_bybusyness.la modules/proxy/balancers/libmod_lbmethod_heartbeat.la modules/arch/unix/libmod_unixd.la modules/dav/main/libmod_dav.la modules/generators/libmod_status.la modules/generators/libmod_autoindex.la modules/generators/libmod_info.la modules/generators/libmod_cgid.la modules/dav/fs/libmod_dav_fs.la modules/mappers/libmod_vhost_alias.la modules/mappers/libmod_negotiation.la modules/mappers/libmod_dir.la modules/mappers/libmod_actions.la modules/mappers/libmod_speling.la modules/mappers/libmod_userdir.la modules/mappers/libmod_alias.la modules/mappers/libmod_rewrite.la
SHLIBPATH_VAR = DYLD_LIBRARY_PATH
OS_SPECIFIC_VARS =
PRE_SHARED_CMDS = echo ""
POST_SHARED_CMDS = echo ""
shared_build =
INSTALL_SUEXEC = setuid
AP_LIBS = $(MOD_AUTHN_FILE_LDADD) $(MOD_AUTHN_DBM_LDADD) $(MOD_AUTHN_ANON_LDADD) $(MOD_AUTHN_DBD_LDADD) $(MOD_AUTHN_SOCACHE_LDADD) $(MOD_AUTHN_CORE_LDADD) $(MOD_AUTHZ_HOST_LDADD) $(MOD_AUTHZ_GROUPFILE_LDADD) $(MOD_AUTHZ_USER_LDADD) $(MOD_AUTHZ_DBM_LDADD) $(MOD_AUTHZ_OWNER_LDADD) $(MOD_AUTHZ_DBD_LDADD) $(MOD_AUTHZ_CORE_LDADD) $(MOD_ACCESS_COMPAT_LDADD) $(MOD_AUTH_BASIC_LDADD) $(MOD_AUTH_FORM_LDADD) $(MOD_AUTH_DIGEST_LDADD) $(MOD_ALLOWMETHODS_LDADD) $(MOD_APREQ_LDADD) $(MOD_FILE_CACHE_LDADD) $(MOD_CACHE_LDADD) $(MOD_CACHE_DISK_LDADD) $(MOD_SOCACHE_SHMCB_LDADD) $(MOD_SOCACHE_DBM_LDADD) $(MOD_SOCACHE_MEMCACHE_LDADD) $(MOD_SO_LDADD) $(MOD_DBD_LDADD) $(MOD_DUMPIO_LDADD) $(MOD_FIREHOSE_LDADD) $(MOD_BUFFER_LDADD) $(MOD_RATELIMIT_LDADD) $(MOD_REQTIMEOUT_LDADD) $(MOD_EXT_FILTER_LDADD) $(MOD_REQUEST_LDADD) $(MOD_INCLUDE_LDADD) $(MOD_FILTER_LDADD) $(MOD_SUBSTITUTE_LDADD) $(MOD_SED_LDADD) $(MOD_DEFLATE_LDADD) $(MOD_HTTP_LDADD) $(MOD_MIME_LDADD) $(MOD_LOG_CONFIG_LDADD) $(MOD_LOG_DEBUG_LDADD) $(MOD_LOGIO_LDADD) $(MOD_ENV_LDADD) $(MOD_EXPIRES_LDADD) $(MOD_HEADERS_LDADD) $(MOD_UNIQUE_ID_LDADD) $(MOD_SETENVIF_LDADD) $(MOD_VERSION_LDADD) $(MOD_REMOTEIP_LDADD) $(MOD_PROXY_LDADD) $(MOD_PROXY_CONNECT_LDADD) $(MOD_PROXY_FTP_LDADD) $(MOD_PROXY_HTTP_LDADD) $(MOD_PROXY_FCGI_LDADD) $(MOD_PROXY_SCGI_LDADD) $(MOD_PROXY_AJP_LDADD) $(MOD_PROXY_BALANCER_LDADD) $(MOD_PROXY_EXPRESS_LDADD) $(MOD_SESSION_LDADD) $(MOD_SESSION_COOKIE_LDADD) $(MOD_SESSION_DBD_LDADD) $(MOD_SLOTMEM_SHM_LDADD) $(MOD_SSL_LDADD) $(MOD_LBMETHOD_BYREQUESTS_LDADD) $(MOD_LBMETHOD_BYTRAFFIC_LDADD) $(MOD_LBMETHOD_BYBUSYNESS_LDADD) $(MOD_LBMETHOD_HEARTBEAT_LDADD) $(MOD_MPM_EVENT_LDADD) $(MOD_UNIXD_LDADD) $(MOD_DAV_LDADD) $(MOD_STATUS_LDADD) $(MOD_AUTOINDEX_LDADD) $(MOD_INFO_LDADD) $(MOD_CGID_LDADD) $(MOD_DAV_FS_LDADD) $(MOD_VHOST_ALIAS_LDADD) $(MOD_NEGOTIATION_LDADD) $(MOD_DIR_LDADD) $(MOD_ACTIONS_LDADD) $(MOD_SPELING_LDADD) $(MOD_USERDIR_LDADD) $(MOD_ALIAS_LDADD) $(MOD_REWRITE_LDADD) /Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr-util/libaprutil-1.la -lexpat /Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr/libapr-1.la -lpthread
AP_BUILD_SRCLIB_DIRS = apr apr-util
AP_CLEAN_SRCLIB_DIRS = apr-util apr
abs_srcdir = /Users/jim/src/asf/code/dev/httpd-trunk
bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/bin
cgidir = ${datadir}/cgi-bin
logfiledir = ${localstatedir}/logs
exec_prefix = ${prefix}
datadir = ${prefix}
localstatedir = ${prefix}
mandir = ${prefix}/man
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/modules
htdocsdir = ${datadir}/htdocs
manualdir = ${datadir}/manual
includedir = ${prefix}/include
errordir = ${datadir}/error
iconsdir = ${datadir}/icons
sysconfdir = ${prefix}/conf
installbuilddir = ${datadir}/build
runtimedir = ${localstatedir}/logs
proxycachedir = ${localstatedir}/proxy
other_targets =
progname = httpd
prefix = /usr/local2/apache2
AWK = gawk
CC = gcc -std=gnu99
CPP = gcc -E
CXX =
CPPFLAGS =
CFLAGS =
CXXFLAGS =
CC_FOR_BUILD =
CFLAGS_FOR_BUILD =
LTFLAGS = --silent
LDFLAGS =
LT_LDFLAGS =
SH_LDFLAGS =
HTTPD_LDFLAGS = -export-dynamic
UTIL_LDFLAGS =
LIBS =
DEFS =
INCLUDES =
NOTEST_CPPFLAGS =
NOTEST_CFLAGS =
NOTEST_CXXFLAGS =
NOTEST_LDFLAGS =
NOTEST_LIBS =
EXTRA_CPPFLAGS = -DDARWIN -DSIGPROCMASK_SETS_THREAD_MASK -no-cpp-precomp -DDARWIN_10 -DAP_DEBUG
EXTRA_CFLAGS = -g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Wpointer-arith -Wdeclaration-after-statement -Werror=declaration-after-statement -Wformat -Wformat-security -Werror=format-security
EXTRA_CXXFLAGS =
EXTRA_LDFLAGS =
EXTRA_LIBS =
EXTRA_INCLUDES = -I. -I$(top_srcdir)/os/$(OS_DIR) -I$(top_srcdir)/include -I/Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr/include -I/Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr-util/include -I/usr/local/include -I$(top_srcdir)/modules/aaa -I$(top_srcdir)/modules/cache -I$(top_srcdir)/modules/core -I$(top_srcdir)/modules/database -I$(top_srcdir)/modules/filters -I$(top_srcdir)/modules/ldap -I$(top_srcdir)/modules/loggers -I$(top_srcdir)/modules/lua -I$(top_srcdir)/modules/proxy -I$(top_srcdir)/modules/session -I$(top_srcdir)/modules/ssl -I$(top_srcdir)/modules/test -I$(top_srcdir)/server -I$(top_srcdir)/modules/arch/unix -I$(top_srcdir)/modules/dav/main -I$(top_srcdir)/modules/generators -I$(top_srcdir)/modules/mappers
INTERNAL_CPPFLAGS =
LIBTOOL = /Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr/libtool $(LTFLAGS)
SHELL = /bin/sh
RSYNC = /usr/local/bin/rsync
MODULE_DIRS = aaa apreq cache core database debugging filters http loggers metadata proxy session slotmem ssl proxy/balancers arch/unix dav/main generators dav/fs mappers
MODULE_CLEANDIRS = arch/win32 echo examples experimental ldap lua test cluster dav/lock
PORT = 80
SSLPORT = 443
CORE_IMPLIB_FILE =
CORE_IMPLIB =
SH_LIBS =
SH_LIBTOOL = $(LIBTOOL)
MK_IMPLIB =
MKDEP = $(CC) -MM
INSTALL_PROG_FLAGS =
DSO_MODULES =
ENABLED_DSO_MODULES =
LOAD_ALL_MODULES = yes
APR_BINDIR = /usr/local2/apache2/bin
APR_INCLUDEDIR = /Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr/include
APR_VERSION = 1.4.7
APR_CONFIG = /usr/local2/apache2/bin/apr-1-config
APU_BINDIR = /usr/local2/apache2/bin
APU_INCLUDEDIR = /Users/jim/src/asf/code/dev/httpd-trunk/srclib/apr-util/include
APU_VERSION = 1.4.3
APU_CONFIG = /usr/local2/apache2/bin/apu-1-config
APREQ_MAJOR_VERSION =
APREQ_LIBTOOL_VERSION =
