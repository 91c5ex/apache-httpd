/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2001 The Apache Software Foundation.  All rights
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

/* User Tracking Module (Was mod_cookies.c)
 *
 * This Apache module is designed to track users paths through a site.
 * It uses the client-side state ("Cookie") protocol developed by Netscape.
 * It is known to work on Netscape browsers, Microsoft Internet
 * Explorer and others currently being developed.
 *
 * Each time a page is requested we look to see if the browser is sending
 * us a Cookie: header that we previously generated.
 *
 * If we don't find one then the user hasn't been to this site since
 * starting their browser or their browser doesn't support cookies.  So
 * we generate a unique Cookie for the transaction and send it back to
 * the browser (via a "Set-Cookie" header)
 * Future requests from the same browser should keep the same Cookie line.
 *
 * By matching up all the requests with the same cookie you can
 * work out exactly what path a user took through your site.  To log
 * the cookie use the " %{Cookie}n " directive in a custom access log;
 *
 * Example 1 : If you currently use the standard Log file format (CLF)
 * and use the command "TransferLog somefilename", add the line
 *       LogFormat "%h %l %u %t \"%r\" %s %b %{Cookie}n"
 * to your config file.
 *
 * Example 2 : If you used to use the old "CookieLog" directive, you
 * can emulate it by adding the following command to your config file
 *       CustomLog filename "%{Cookie}n \"%r\" %t"
 *
 * Notes:
 * 1.  This code now logs the initial transaction (the one that created
 *     the cookie to start with).
 * 2.  This module has been designed to not interfere with other Cookies
 *     your site may be using; just avoid sending out cookies with
 *     the name "Apache=" or things will get confused.
 * 3.  If you want you can modify the Set-Cookie line so that the Cookie
 *     never expires.  You would then get the same Cookie each time the
 *     user revisits your site.
 *
 * Mark Cox, mark@ukweb.com, 6 July 95
 *
 * This file replaces mod_cookies.c
 */

#include "apr.h"
#include "apr_lib.h"
#include "apr_strings.h"

#define APR_WANT_STRFUNC
#include "apr_want.h"

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_request.h"


module AP_MODULE_DECLARE_DATA usertrack_module;

typedef struct {
    int always;
    int expires;
} cookie_log_state;

typedef enum {
    CT_UNSET,
    CT_NETSCAPE,
    CT_COOKIE,
    CT_COOKIE2
} cookie_type_e;

typedef struct {
    int enabled;
    cookie_type_e style;
    char *cookie_name;
    char *cookie_domain;
} cookie_dir_rec;

/* Make Cookie: Now we have to generate something that is going to be
 * pretty unique.  We can base it on the pid, time, hostip */

#define COOKIE_NAME "Apache"

static void make_cookie(request_rec *r)
{
    cookie_log_state *cls = ap_get_module_config(r->server->module_config,
						 &usertrack_module);
    /* 1024 == hardcoded constant */
    char cookiebuf[1024];
    char *new_cookie;
    const char *rname = ap_get_remote_host(r->connection, r->per_dir_config,
					   REMOTE_NAME, NULL);
    cookie_dir_rec *dcfg;

    dcfg = ap_get_module_config(r->per_dir_config, &usertrack_module);

    /* XXX: hmm, this should really tie in with mod_unique_id */
    apr_snprintf(cookiebuf, sizeof(cookiebuf), "%s.%qd", rname, apr_time_now());

    if (cls->expires) {
	apr_exploded_time_t tms;
        time_t when;

        when = cls->expires;
        if ((dcfg->style == CT_UNSET) || (dcfg->style == CT_NETSCAPE)) {
            when += r->request_time;
        }

	apr_explode_gmt(&tms,
                        r->request_time + cls->expires * APR_USEC_PER_SEC);

        /* Cookie with date; as strftime '%a, %d-%h-%y %H:%M:%S GMT' */
        new_cookie = apr_psprintf(r->pool, "%s=%s; path=/",
                                  dcfg->cookie_name, cookiebuf);
        if ((dcfg->style == CT_UNSET) || (dcfg->style == CT_NETSCAPE)) {
            new_cookie = apr_psprintf(r->pool,
                                       "%s; expires=%s, "
                                       "%.2d-%s-%.2d %.2d:%.2d:%.2d GMT",
                                       new_cookie, apr_day_snames[tms.tm_wday],
                                       tms.tm_mday,
                                       apr_month_snames[tms.tm_mon],
                                       tms.tm_year % 100,
                                       tms.tm_hour, tms.tm_min, tms.tm_sec);
        }
        else {
            new_cookie = apr_psprintf(r->pool, "%s; max-age=%d",
                                      new_cookie, (int) when);
        }
    }
    else {
        new_cookie = apr_psprintf(r->pool, "%s=%s; path=/",
                                  dcfg->cookie_name, cookiebuf);
    }
    if (dcfg->cookie_domain != NULL) {
        new_cookie = apr_pstrcat(r->pool, new_cookie, "; domain=",
                                 dcfg->cookie_domain,
                                 (dcfg->style == CT_COOKIE2
                                  ? "; version=1"
                                  : ""),
                                 NULL);
    }

    apr_table_setn(r->headers_out,
                   (dcfg->style == CT_COOKIE2 ? "Set-Cookie2" : "Set-Cookie"),
                   new_cookie);
    apr_table_setn(r->notes, "cookie", apr_pstrdup(r->pool, cookiebuf));   /* log first time */
    return;
}

static int spot_cookie(request_rec *r)
{
    cookie_dir_rec *dcfg = ap_get_module_config(r->per_dir_config,
						&usertrack_module);
    const char *cookie;
    const char *value;

    if (!dcfg->enabled) {
        return DECLINED;
    }

    if ((cookie = apr_table_get(r->headers_in,
                                (dcfg->style == CT_COOKIE2
                                 ? "Cookie2"
                                 : "Cookie"))))
        if ((value = ap_strstr_c(cookie, dcfg->cookie_name))) {
            char *cookiebuf, *cookieend;

            value += strlen(dcfg->cookie_name) + 1;  /* Skip over the '=' */
            cookiebuf = apr_pstrdup(r->pool, value);
            cookieend = strchr(cookiebuf, ';');
            if (cookieend)
                *cookieend = '\0';      /* Ignore anything after a ; */

            /* Set the cookie in a note, for logging */
            apr_table_setn(r->notes, "cookie", cookiebuf);

            return DECLINED;    /* There's already a cookie, no new one */
        }
    make_cookie(r);
    return OK;                  /* We set our cookie */
}

static void *make_cookie_log_state(apr_pool_t *p, server_rec *s)
{
    cookie_log_state *cls =
    (cookie_log_state *) apr_palloc(p, sizeof(cookie_log_state));

    cls->expires = 0;

    return (void *) cls;
}

static void *make_cookie_dir(apr_pool_t *p, char *d)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) apr_pcalloc(p, sizeof(cookie_dir_rec));
    dcfg->cookie_name = COOKIE_NAME;
    dcfg->cookie_domain = NULL;
    dcfg->style = CT_UNSET;
    dcfg->enabled = 0;
    return dcfg;
}

static const char *set_cookie_enable(cmd_parms *cmd, void *mconfig, int arg)
{
    cookie_dir_rec *dcfg = mconfig;

    dcfg->enabled = arg;
    return NULL;
}

static const char *set_cookie_exp(cmd_parms *parms, void *dummy,
                                  const char *arg)
{
    cookie_log_state *cls;
    time_t factor, modifier = 0;
    time_t num = 0;
    char *word;

    cls  = ap_get_module_config(parms->server->module_config,
                                &usertrack_module);
    /* The simple case first - all numbers (we assume) */
    if (apr_isdigit(arg[0]) && apr_isdigit(arg[strlen(arg) - 1])) {
        cls->expires = atol(arg);
        return NULL;
    }

    /*
     * The harder case - stolen from mod_expires 
     *
     * CookieExpires "[plus] {<num> <type>}*"
     */

    word = ap_getword_conf(parms->pool, &arg);
    if (!strncasecmp(word, "plus", 1)) {
        word = ap_getword_conf(parms->pool, &arg);
    };

    /* {<num> <type>}* */
    while (word[0]) {
        /* <num> */
	if (apr_isdigit(word[0]))
            num = atoi(word);
        else
            return "bad expires code, numeric value expected.";

        /* <type> */
        word = ap_getword_conf(parms->pool, &arg);
        if (!word[0])
            return "bad expires code, missing <type>";

        factor = 0;
        if (!strncasecmp(word, "years", 1))
            factor = 60 * 60 * 24 * 365;
        else if (!strncasecmp(word, "months", 2))
            factor = 60 * 60 * 24 * 30;
        else if (!strncasecmp(word, "weeks", 1))
            factor = 60 * 60 * 24 * 7;
        else if (!strncasecmp(word, "days", 1))
            factor = 60 * 60 * 24;
        else if (!strncasecmp(word, "hours", 1))
            factor = 60 * 60;
        else if (!strncasecmp(word, "minutes", 2))
            factor = 60;
        else if (!strncasecmp(word, "seconds", 1))
            factor = 1;
        else
            return "bad expires code, unrecognized type";

        modifier = modifier + factor * num;

        /* next <num> */
        word = ap_getword_conf(parms->pool, &arg);
    }

    cls->expires = modifier;

    return NULL;
}

static const char *set_cookie_name(cmd_parms *cmd, void *mconfig,
                                   const char *name)
{
    cookie_dir_rec *dcfg = (cookie_dir_rec *) mconfig;

    dcfg->cookie_name = apr_pstrdup(cmd->pool, name);
    return NULL;
}

/*
 * Set the value for the 'Domain=' attribute.
 */
static const char *set_cookie_domain(cmd_parms *cmd, void *mconfig,
                                     const char *name)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) mconfig;

    /*
     * Apply the restrictions on cookie domain attributes.
     */
    if (strlen(name) == 0) {
        return "CookieDomain values may not be null";
    }
    if (name[0] != '.') {
        return "CookieDomain values must begin with a dot";
    }
    if (strchr(&name[1], '.') == NULL) {
        return "CookieDomain values must contain at least one embedded dot";
    }

    dcfg->cookie_domain = apr_pstrdup(cmd->pool, name);
    return NULL;
}

/*
 * Make a note of the cookie style we should use.
 */
static const char *set_cookie_style(cmd_parms *cmd, void *mconfig,
                                    const char *name)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) mconfig;

    if (strcasecmp(name, "Netscape") == 0) {
        dcfg->style = CT_NETSCAPE;
    }
    else if ((strcasecmp(name, "Cookie") == 0)
             || (strcasecmp(name, "RFC2109") == 0)) {
        dcfg->style = CT_COOKIE;
    }
    else if ((strcasecmp(name, "Cookie2") == 0)
             || (strcasecmp(name, "RFC2965") == 0)) {
        dcfg->style = CT_COOKIE2;
    }
    else {
        return apr_psprintf(cmd->pool, "Invalid %s keyword: '%s'",
                            cmd->cmd->name, name);
    }

    return NULL;
}

static const command_rec cookie_log_cmds[] = {
    AP_INIT_TAKE1("CookieExpires", set_cookie_exp, NULL, OR_FILEINFO,
                  "an expiry date code"),
    AP_INIT_TAKE1("CookieDomain", set_cookie_domain, NULL, OR_FILEINFO,
                  "domain to which this cookie applies"),
    AP_INIT_TAKE1("CookieStyle", set_cookie_style, NULL, OR_FILEINFO,
                  "'Netscape', 'Cookie' (RFC2109), or 'Cookie2' (RFC2965)"),
    AP_INIT_FLAG("CookieTracking", set_cookie_enable, NULL, OR_FILEINFO,
                 "whether or not to enable cookies"),
    AP_INIT_TAKE1("CookieName", set_cookie_name, NULL, OR_FILEINFO,
                  "name of the tracking cookie"),
    {NULL}
};

static void register_hooks(apr_pool_t *p)
{
    ap_hook_fixups(spot_cookie,NULL,NULL,APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA usertrack_module = {
    STANDARD20_MODULE_STUFF,
    make_cookie_dir,            /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    make_cookie_log_state,      /* server config */
    NULL,                       /* merge server configs */
    cookie_log_cmds,            /* command apr_table_t */
    register_hooks		/* register hooks */
};
