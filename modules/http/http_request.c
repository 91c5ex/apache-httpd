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

/*
 * http_request.c: functions to get and process requests
 *
 * Rob McCool 3/21/93
 *
 * Thoroughly revamped by rst for Apache.  NB this file reads
 * best from the bottom up.
 *
 */

#include "apr_strings.h"
#include "apr_file_io.h"
#include "apr_fnmatch.h"

#define APR_WANT_STRFUNC
#include "apr_want.h"

#define CORE_PRIVATE
#include "ap_config.h"
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_filter.h"
#include "util_charset.h"

#include "mod_core.h"

#if APR_HAVE_STDARG_H
#include <stdarg.h>
#endif

static void add_required_filters(request_rec *r)
{
    ap_filter_t *f = r->output_filters;
    int has_core = 0, has_content = 0, has_http_header = 0;
    while (f) {
        if(!strcasecmp(f->frec->name, "CORE"))
            has_core = 1; 
        else if(!strcasecmp(f->frec->name, "CONTENT_LENGTH"))
            has_content = 1; 
        else if(!strcasecmp(f->frec->name, "HTTP_HEADER")) 
            has_http_header = 1;
        f = f->next;
    }
    if(!has_core) 
        ap_add_output_filter("CORE", NULL, r, r->connection);
    if(!has_content)
        ap_add_output_filter("CONTENT_LENGTH", NULL, r, r->connection);
    if(!has_http_header) 
        ap_add_output_filter("HTTP_HEADER", NULL, r, r->connection);

}

/*****************************************************************
 *
 * Mainline request processing...
 */

AP_DECLARE(void) ap_die(int type, request_rec *r)
{
    int error_index = ap_index_of_response(type);
    char *custom_response = ap_response_code_string(r, error_index);
    int recursive_error = 0;

    if (type == AP_FILTER_ERROR) {
        return;
    }

    if (type == DONE) {
        ap_finalize_request_protocol(r);
        return;
    }

    /*
     * The following takes care of Apache redirects to custom response URLs
     * Note that if we are already dealing with the response to some other
     * error condition, we just report on the original error, and give up on
     * any attempt to handle the other thing "intelligently"...
     */

    if (r->status != HTTP_OK) {
        recursive_error = type;

        while (r->prev && (r->prev->status != HTTP_OK))
            r = r->prev;        /* Get back to original error */

        type = r->status;
        custom_response = NULL; /* Do NOT retry the custom thing! */
    }

    r->status = type;

    /*
     * This test is done here so that none of the auth modules needs to know
     * about proxy authentication.  They treat it like normal auth, and then
     * we tweak the status.
     */
    if (HTTP_UNAUTHORIZED == r->status && PROXYREQ_PROXY == r->proxyreq) {
        r->status = HTTP_PROXY_AUTHENTICATION_REQUIRED;
    }

    /*
     * If we want to keep the connection, be sure that the request body
     * (if any) has been read.
     */
    if ((r->status != HTTP_NOT_MODIFIED) && (r->status != HTTP_NO_CONTENT)
        && !ap_status_drops_connection(r->status)
        && r->connection && (r->connection->keepalive != -1)) {

        (void) ap_discard_request_body(r);
    }

    /*
     * Two types of custom redirects --- plain text, and URLs. Plain text has
     * a leading '"', so the URL code, here, is triggered on its absence
     */

    if (custom_response && custom_response[0] != '"') {

        if (ap_is_url(custom_response)) {
            /*
             * The URL isn't local, so lets drop through the rest of this
             * apache code, and continue with the usual REDIRECT handler.
             * But note that the client will ultimately see the wrong
             * status...
             */
            r->status = HTTP_MOVED_TEMPORARILY;
            apr_table_setn(r->headers_out, "Location", custom_response);
        }
        else if (custom_response[0] == '/') {
            const char *error_notes;
            r->no_local_copy = 1;       /* Do NOT send HTTP_NOT_MODIFIED for
                                         * error documents! */
            /*
             * This redirect needs to be a GET no matter what the original
             * method was.
             */
            apr_table_setn(r->subprocess_env, "REQUEST_METHOD", r->method);

	    /*
	     * Provide a special method for modules to communicate
	     * more informative (than the plain canned) messages to us.
	     * Propagate them to ErrorDocuments via the ERROR_NOTES variable:
	     */
            if ((error_notes = apr_table_get(r->notes, "error-notes")) != NULL) {
		apr_table_setn(r->subprocess_env, "ERROR_NOTES", error_notes);
	    }
            r->method = apr_pstrdup(r->pool, "GET");
            r->method_number = M_GET;
            ap_internal_redirect(custom_response, r);
            return;
        }
        else {
            /*
             * Dumb user has given us a bad url to redirect to --- fake up
             * dying with a recursive server error...
             */
            recursive_error = HTTP_INTERNAL_SERVER_ERROR;
            ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, r,
                        "Invalid error redirection directive: %s",
                        custom_response);
        }
    }
    add_required_filters(r);
    ap_send_error_response(r, recursive_error);
}

static void check_pipeline_flush(request_rec *r)
{
    /* ### if would be nice if we could PEEK without a brigade. that would
       ### allow us to defer creation of the brigade to when we actually
       ### need to send a FLUSH. */
    apr_bucket_brigade *bb = apr_brigade_create(r->pool);
    apr_off_t zero = 0;

    /* Flush the filter contents if:
     *
     *   1) the connection will be closed
     *   2) there isn't a request ready to be read
     */
    /* ### shouldn't this read from the connection input filters? */
    /* ### is zero correct? that means "read one line" */
    if (!r->connection->keepalive || 
        ap_get_brigade(r->input_filters, bb, AP_MODE_PEEK, &zero) != APR_SUCCESS) {
        apr_bucket *e = apr_bucket_flush_create();

        /* We just send directly to the connection based filters.  At
         * this point, we know that we have seen all of the data
         * (request finalization sent an EOS bucket, which empties all
         * of the request filters). We just want to flush the buckets
         * if something hasn't been sent to the network yet.
         */
        APR_BRIGADE_INSERT_HEAD(bb, e);
        ap_pass_brigade(r->connection->output_filters, bb);
    }
}

void ap_process_request(request_rec *r)
{
    int access_status;

    /* Give quick handlers a shot at serving the request on the fast
     * path, bypassing all of the other Apache hooks. 
     *
     * This hook was added to enable serving files out of a URI keyed 
     * content cache ( e.g., Mike Abbott's Quick Shortcut Cache, 
     * described here: http://oss.sgi.com/projects/apache/mod_qsc.html )
     *
     * It may have other uses as well, such as routing requests directly to
     * content handlers that have the ability to grok HTTP and do their
     * own access checking, etc (e.g. servlet engines). 
     * 
     * Use this hook with extreme care and only if you know what you are 
     * doing.
     * 
     * Consider moving this hook to after the first location_walk in order
     * to enable the quick handler to make decisions based on config
     * directives in Location blocks.
     */
    access_status = ap_run_quick_handler(r);
    if (access_status == DECLINED) {
        access_status = ap_process_request_internal(r);
        if (access_status == OK)
            access_status = ap_invoke_handler(r);
    }

    if (access_status == OK) {
        ap_finalize_request_protocol(r);
    }
    else {
        ap_die(access_status, r);
    }
    
    /*
     * We want to flush the last packet if this isn't a pipelining connection
     * *before* we start into logging.  Suppose that the logging causes a DNS
     * lookup to occur, which may have a high latency.  If we hold off on
     * this packet, then it'll appear like the link is stalled when really
     * it's the application that's stalled.
     */
    check_pipeline_flush(r);
    ap_run_log_transaction(r);
}

static apr_table_t *rename_original_env(apr_pool_t *p, apr_table_t *t)
{
    apr_array_header_t *env_arr = apr_table_elts(t);
    apr_table_entry_t *elts = (apr_table_entry_t *) env_arr->elts;
    apr_table_t *new = apr_table_make(p, env_arr->nalloc);
    int i;

    for (i = 0; i < env_arr->nelts; ++i) {
        if (!elts[i].key)
            continue;
        apr_table_setn(new, apr_pstrcat(p, "REDIRECT_", elts[i].key, NULL),
                  elts[i].val);
    }

    return new;
}

static request_rec *internal_internal_redirect(const char *new_uri,
					       request_rec *r) {
    int access_status;
    request_rec *new = (request_rec *) apr_pcalloc(r->pool,
						   sizeof(request_rec));

    new->connection = r->connection;
    new->server     = r->server;
    new->pool       = r->pool;

    /*
     * A whole lot of this really ought to be shared with http_protocol.c...
     * another missing cleanup.  It's particularly inappropriate to be
     * setting header_only, etc., here.
     */

    new->method          = r->method;
    new->method_number   = r->method_number;
    new->allowed_methods = ap_make_method_list(new->pool, 2);
    ap_parse_uri(new, new_uri);

    new->request_config = ap_create_request_config(r->pool);

    new->per_dir_config = r->server->lookup_defaults;

    new->prev = r;
    r->next   = new;

    /* Must have prev and next pointers set before calling create_request
     * hook.
     */
    ap_run_create_request(new);

    /* Inherit the rest of the protocol info... */

    new->the_request = r->the_request;

    new->allowed         = r->allowed;

    new->status          = r->status;
    new->assbackwards    = r->assbackwards;
    new->header_only     = r->header_only;
    new->protocol        = r->protocol;
    new->proto_num       = r->proto_num;
    new->hostname        = r->hostname;
    new->request_time    = r->request_time;
    new->main            = r->main;

    new->headers_in      = r->headers_in;
    new->headers_out     = apr_table_make(r->pool, 12);
    new->err_headers_out = r->err_headers_out;
    new->subprocess_env  = rename_original_env(r->pool, r->subprocess_env);
    new->notes           = apr_table_make(r->pool, 5);
    new->allowed_methods = ap_make_method_list(new->pool, 2);

    new->htaccess        = r->htaccess;
    new->no_cache        = r->no_cache;
    new->expecting_100	 = r->expecting_100;
    new->no_local_copy   = r->no_local_copy;
    new->read_length     = r->read_length;     /* We can only read it once */
    new->vlist_validator = r->vlist_validator;

    new->output_filters  = r->connection->output_filters;
    new->input_filters   = r->connection->input_filters;

    apr_table_setn(new->subprocess_env, "REDIRECT_STATUS",
	apr_psprintf(r->pool, "%d", r->status));

    /*
     * XXX: hmm.  This is because mod_setenvif and mod_unique_id really need
     * to do their thing on internal redirects as well.  Perhaps this is a
     * misnamed function.
     */
    if ((access_status = ap_run_post_read_request(new))) {
        ap_die(access_status, new);
        return NULL;
    }

    return new;
}

/* XXX: Is this function is so bogus and fragile that we deep-6 it? */
AP_DECLARE(void) ap_internal_fast_redirect(request_rec *rr, request_rec *r)
{
    /* We need to tell POOL_DEBUG that we're guaranteeing that rr->pool
     * will exist as long as r->pool.  Otherwise we run into troubles because
     * some values in this request will be allocated in r->pool, and others in
     * rr->pool.
     */
    apr_pool_join(r->pool, rr->pool);
    r->mtime = rr->mtime;
    r->uri = rr->uri;
    r->args = rr->args;
    r->filename = rr->filename;
    r->canonical_filename = rr->canonical_filename;
    r->handler = rr->handler;
    r->content_type = rr->content_type;
    r->content_encoding = rr->content_encoding;
    r->content_languages = rr->content_languages;
    r->content_language = rr->content_language;
    r->finfo = rr->finfo;
    r->per_dir_config = rr->per_dir_config;
    /* copy output headers from subrequest, but leave negotiation headers */
    r->notes = apr_table_overlay(r->pool, rr->notes, r->notes);
    r->headers_out = apr_table_overlay(r->pool, rr->headers_out,
                                    r->headers_out);
    r->err_headers_out = apr_table_overlay(r->pool, rr->err_headers_out,
                                        r->err_headers_out);
    r->subprocess_env = apr_table_overlay(r->pool, rr->subprocess_env,
                                       r->subprocess_env);
}

AP_DECLARE(void) ap_internal_redirect(const char *new_uri, request_rec *r)
{
    request_rec *new = internal_internal_redirect(new_uri, r);
    int access_status = ap_process_request_internal(new);
    if (access_status == OK) {
        if ((access_status = ap_invoke_handler(new)) != 0) {
            ap_die(access_status, new);
            return;
        }
        ap_finalize_request_protocol(new);
    }
    else {
        ap_die(access_status, new);
    }
}

/* This function is designed for things like actions or CGI scripts, when
 * using AddHandler, and you want to preserve the content type across
 * an internal redirect.
 */
AP_DECLARE(void) ap_internal_redirect_handler(const char *new_uri, request_rec *r)
{
    int access_status;
    request_rec *new = internal_internal_redirect(new_uri, r);
    if (r->handler)
        new->content_type = r->content_type;
    access_status = ap_process_request_internal(new);
    if (access_status == OK) {
        if ((access_status = ap_invoke_handler(new)) != 0) {
            ap_die(access_status, new);
            return;
        }
        ap_finalize_request_protocol(new);
    }
    else {
        ap_die(access_status, new);
    }
}

AP_DECLARE(void) ap_allow_methods(request_rec *r, int reset, ...) 
{
    const char *method;
    va_list methods;

    /*
     * Get rid of any current settings if requested; not just the
     * well-known methods but any extensions as well.
     */
    if (reset) {
	ap_clear_method_list(r->allowed_methods);
    }

    va_start(methods, reset);
    while ((method = va_arg(methods, const char *)) != NULL) {
	ap_method_list_add(r->allowed_methods, method);
    }
}

AP_DECLARE(void) ap_allow_standard_methods(request_rec *r, int reset, ...)
{
    int method;
    va_list methods;
    apr_int64_t mask;

    /*
     * Get rid of any current settings if requested; not just the
     * well-known methods but any extensions as well.
     */
    if (reset) {
        ap_clear_method_list(r->allowed_methods);
    }

    mask = 0;
    va_start(methods, reset);
    while ((method = va_arg(methods, int)) != -1) {
        mask |= (AP_METHOD_BIT << method);
    }
    va_end(methods);

    r->allowed_methods->method_mask |= mask;
}
