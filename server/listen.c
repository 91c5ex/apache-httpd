/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

#include "apr_network_io.h"

#define CORE_PRIVATE
#include "ap_config.h"
#include "httpd.h"
#include "http_config.h"
#include "ap_listen.h"
#include "apr_strings.h"
#include "http_log.h"
#include "mpm.h"
#include "mpm_common.h"
#ifdef HAVE_STRING_H
#include <string.h>
#endif

ap_listen_rec *ap_listeners;
static ap_listen_rec *old_listeners;
static int ap_listenbacklog;
static int send_buffer_size;

/* TODO: make_sock is just begging and screaming for APR abstraction */
static apr_status_t make_sock(apr_pool_t *p, ap_listen_rec *server)
{
    apr_socket_t *s = server->sd;
    int one = 1;
    apr_status_t stat;

    stat = apr_setsocketopt(s, APR_SO_REUSEADDR, one);
    if (stat != APR_SUCCESS && stat != APR_ENOTIMPL) {
	ap_log_error(APLOG_MARK, APLOG_CRIT, stat, NULL,
		    "make_sock: for address %pI, setsockopt: (SO_REUSEADDR)", 
                     server->bind_addr);
	apr_close_socket(s);
	return stat;
    }
    
    stat = apr_setsocketopt(s, APR_SO_KEEPALIVE, one);
    if (stat != APR_SUCCESS && stat != APR_ENOTIMPL) {
	ap_log_error(APLOG_MARK, APLOG_CRIT, stat, NULL,
		    "make_sock: for address %pI, setsockopt: (SO_KEEPALIVE)", 
                     server->bind_addr);
	apr_close_socket(s);
	return stat;
    }

    /*
     * To send data over high bandwidth-delay connections at full
     * speed we must force the TCP window to open wide enough to keep the
     * pipe full.  The default window size on many systems
     * is only 4kB.  Cross-country WAN connections of 100ms
     * at 1Mb/s are not impossible for well connected sites.
     * If we assume 100ms cross-country latency,
     * a 4kB buffer limits throughput to 40kB/s.
     *
     * To avoid this problem I've added the SendBufferSize directive
     * to allow the web master to configure send buffer size.
     *
     * The trade-off of larger buffers is that more kernel memory
     * is consumed.  YMMV, know your customers and your network!
     *
     * -John Heidemann <johnh@isi.edu> 25-Oct-96
     *
     * If no size is specified, use the kernel default.
     */
    if (send_buffer_size) {
	stat = apr_setsocketopt(s, APR_SO_SNDBUF,  send_buffer_size);
        if (stat != APR_SUCCESS && stat != APR_ENOTIMPL) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, stat, NULL,
			"make_sock: failed to set SendBufferSize for "
                         "address %pI, using default", 
                         server->bind_addr);
	    /* not a fatal error */
	}
    }

#if DISABLE_NAGLE_INHERITED
    ap_sock_disable_nagle(s);
#endif

    if ((stat = apr_bind(s, server->bind_addr)) != APR_SUCCESS) {
	ap_log_error(APLOG_MARK, APLOG_CRIT, stat, NULL,
                     "make_sock: could not bind to address %pI", 
                     server->bind_addr);
	apr_close_socket(s);
	return stat;
    }

    if ((stat = apr_listen(s, ap_listenbacklog)) != APR_SUCCESS) {
	ap_log_error(APLOG_MARK, APLOG_ERR, stat, NULL,
	    "make_sock: unable to listen for connections on address %pI", 
                     server->bind_addr);
	apr_close_socket(s);
	return stat;
    }

    server->sd = s;
    server->active = 1;
    return APR_SUCCESS;
}


static apr_status_t close_listeners_on_exec(void *v)
{
    ap_listen_rec *lr;

    for (lr = ap_listeners; lr; lr = lr->next) {
	apr_close_socket(lr->sd);
	lr->active = 0;
    }
    return APR_SUCCESS;
}


static void alloc_listener(process_rec *process, char *addr, apr_port_t port)
{
    ap_listen_rec **walk;
    ap_listen_rec *new;
    apr_status_t status;
    char *oldaddr;
    apr_port_t oldport;
    apr_sockaddr_t *sa;

    if (!addr) {
        /* XXX not valid for IPv6 if we can get an IPv6 socket when the
         *     config doesn't specify an interface address
         *
         *     We need to look at the configuration to see which mode we're
         *     in: 
         *     a) get IPv6 if we can but fall back to IPv4
         *        => leave addr NULL so the logic below calls apr_create_socket()
         *           first
         *     b) get IPv4
         *        => set addr to 0.0.0.0
         *     c) get IPv6
         *        => set addr to ::
         *
         *     Alternative: earlier in initialization, if we start up in fall-
         *     back-to-IPv4 mode, go ahead and see if we can get an IPv6 socket;
         *     if not, set mode to get-IPv4; then, we have fewer cases to handle
         *     here (and probably elsewhere)
         */
        addr = APR_ANYADDR;
    }

    /* see if we've got an old listener for this address:port */
    for (walk = &old_listeners; *walk; walk = &(*walk)->next) {
        apr_get_sockaddr(&sa, APR_LOCAL, (*walk)->sd);
        apr_get_port(&oldport, sa);
	apr_get_ipaddr(&oldaddr, sa);
	if (!strcmp(oldaddr, addr) && port == oldport) {
	    /* re-use existing record */
	    new = *walk;
	    *walk = new->next;
	    new->next = ap_listeners;
	    ap_listeners = new;
	    return;
	}
    }

    /* this has to survive restarts */
    new = apr_palloc(process->pool, sizeof(ap_listen_rec));
    new->active = 0;
    if (addr) {
        /* binding to specific interface; let apr_getaddrinfo() figure out
         * what address family is appropriate;
         */
        if ((status = apr_getaddrinfo(&new->bind_addr, addr, APR_UNSPEC, port, 0, 
                                      process->pool)) != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_CRIT, status, NULL,
			 "alloc_listener: failed to set up sockaddr for %s", addr);
            return;
        }
        if ((status = apr_create_socket(&new->sd, new->bind_addr->sa.sin.sin_family, 
                                        SOCK_STREAM, process->pool)) != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_CRIT, status, NULL,
                         "alloc_listener: failed to get a socket for %s", addr);
            return;
        }
    }
    else { /* XXX See big XXX above in this function. */
        if ((status = apr_create_socket(&new->sd, APR_INET,
                                        SOCK_STREAM, process->pool)) != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_CRIT, status, NULL,
                         "alloc_listener: failed to get a socket for INADDR_ANY");
            return;
        }
        apr_get_sockaddr(&new->bind_addr, APR_LOCAL, new->sd);
        apr_set_port(new->bind_addr, port);
    }
    new->next = ap_listeners;
    ap_listeners = new;
}

#if !defined(WIN32) && !defined(SPMT_OS2_MPM)
static
#endif
int ap_listen_open(process_rec *process, apr_port_t port)
{
    apr_pool_t *pconf = process->pconf;
    ap_listen_rec *lr;
    ap_listen_rec *next;
    int num_open;

    /* allocate a default listener if necessary */
    if (ap_listeners == NULL) {
	alloc_listener(process, NULL, port ? port : DEFAULT_HTTP_PORT);
    }

    num_open = 0;
    for (lr = ap_listeners; lr; lr = lr->next) {
	if (lr->active) {
	    ++num_open;
	}
	else {
	    if (make_sock(pconf, lr) == APR_SUCCESS) {
		++num_open;
		lr->active = 1;
	    }
	}
    }

    /* close the old listeners */
    for (lr = old_listeners; lr; lr = next) {
	apr_close_socket(lr->sd);
	lr->active = 0;
	next = lr->next;
/*	free(lr);*/
    }
    old_listeners = NULL;

    apr_register_cleanup(pconf, NULL, apr_null_cleanup, close_listeners_on_exec);

    return num_open ? 0 : -1;
}

#if !defined(WIN32)
int ap_setup_listeners(server_rec *s)
{
    ap_listen_rec *lr;
    int num_listeners = 0;
    if (ap_listen_open(s->process, s->port)) {
       return 0;
    }
    for (lr = ap_listeners; lr; lr = lr->next) {
        num_listeners++;
    }
    return num_listeners;
}
#endif

void ap_listen_pre_config(void)
{
    old_listeners = ap_listeners;
    ap_listeners = NULL;
    ap_listenbacklog = DEFAULT_LISTENBACKLOG;
}


const char *ap_set_listener(cmd_parms *cmd, void *dummy, const char *ips)
{
    char *host, *scope_id;
    apr_port_t port;
    apr_status_t rv;

    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    if (err != NULL) {
        return err;
    }

    rv = apr_parse_addr_port(&host, &scope_id, &port, ips, cmd->pool);
    if (rv != APR_SUCCESS) {
        return "Invalid address or port";
    }
    if (host && !strcmp(host, "*")) {
        host = NULL;
    }
    if (scope_id) {
        /* XXX scope id support is useful with link-local IPv6 addresses */
        return "Scope id is not supported";
    }
    if (!port) {
        return "Port must be specified";
    }

    alloc_listener(cmd->server->process, host, port);

    return NULL;
}

const char *ap_set_listenbacklog(cmd_parms *cmd, void *dummy, const char *arg) 
{
    int b;

    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    if (err != NULL) {
        return err;
    }

    b = atoi(arg);
    if (b < 1) {
        return "ListenBacklog must be > 0";
    }
    ap_listenbacklog = b;
    return NULL;
}

const char *ap_set_send_buffer_size(cmd_parms *cmd, void *dummy, const char *arg)
{
    int s = atoi(arg);
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    if (err != NULL) {
        return err;
    }

    if (s < 512 && s != 0) {
        return "SendBufferSize must be >= 512 bytes, or 0 for system default.";
    }
    send_buffer_size = s;
    return NULL;
}
