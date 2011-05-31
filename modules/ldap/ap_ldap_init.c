/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * ap_ldap_init.c: LDAP v2/v3 common initialise
 * 
 * Original code from auth_ldap module for Apache v1.3:
 * Copyright 1998, 1999 Enbridge Pipelines Inc. 
 * Copyright 1999-2001 Dave Carrigan
 */

#include "apr.h"
#include "apu.h"

#include "ap_config.h"
#include "ap_ldap.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_strings.h"

#if AP_HAS_LDAP

/**
 * APR LDAP SSL Initialise function
 *
 * This function initialises SSL on the underlying LDAP toolkit
 * if this is necessary.
 *
 * If a CA certificate is provided, this is set, however the setting
 * of certificates via this method has been deprecated and will be removed in
 * APR v2.0.
 *
 * The ap_ldap_set_option() function with the AP_LDAP_OPT_TLS_CERT option
 * should be used instead to set certificates.
 *
 * If SSL support is not available on this platform, or a problem
 * was encountered while trying to set the certificate, the function
 * will return APR_EGENERAL. Further LDAP specific error information
 * can be found in result_err.
 */
MODLDAP_DECLARE(int) ap_ldap_ssl_init(apr_pool_t *pool,
                                      const char *cert_auth_file,
                                      int cert_file_type,
                                      ap_ldap_err_t **result_err)
{

    ap_ldap_err_t *result = (ap_ldap_err_t *)apr_pcalloc(pool, sizeof(ap_ldap_err_t));
    *result_err = result;

#if AP_HAS_LDAP_SSL /* compiled with ssl support */

    /* Novell */
#if AP_HAS_NOVELL_LDAPSDK
    ldapssl_client_init(NULL, NULL);
#endif

    /* if a certificate was specified, set it */
    if (cert_auth_file) {
        ap_ldap_opt_tls_cert_t *cert = (ap_ldap_opt_tls_cert_t *)apr_pcalloc(pool, sizeof(ap_ldap_opt_tls_cert_t));
        cert->type = cert_file_type;
        cert->path = cert_auth_file;
        return ap_ldap_set_option(pool, NULL, AP_LDAP_OPT_TLS_CERT, (void *)cert, result_err);
    }

#else  /* not compiled with SSL Support */
    if (cert_auth_file) {
        result->reason = "LDAP: Attempt to set certificate store failed. "
                         "Not built with SSL support";
        result->rc = -1;
    }
#endif /* AP_HAS_LDAP_SSL */

    if (result->rc != -1) {
        result->msg = ldap_err2string(result->rc);
    }

    if (LDAP_SUCCESS != result->rc) {
        return APR_EGENERAL;
    }

    return APR_SUCCESS;

} 


/**
 * APR LDAP SSL De-Initialise function
 *
 * This function tears down any SSL certificate setup previously
 * set using ap_ldap_ssl_init(). It should be called to clean
 * up if a graceful restart of a service is attempted.
 *
 * This function only does anything on Netware.
 *
 * @todo currently we do not check whether ap_ldap_ssl_init()
 * has been called first - should we?
 */
MODLDAP_DECLARE(int) ap_ldap_ssl_deinit(void)
{

#if AP_HAS_LDAP_SSL && AP_HAS_LDAPSSL_CLIENT_DEINIT
    ldapssl_client_deinit();
#endif
    return APR_SUCCESS;

}


/**
 * APR LDAP initialise function
 *
 * This function is responsible for initialising an LDAP
 * connection in a toolkit independant way. It does the
 * job of ldap_init() from the C api.
 *
 * It handles both the SSL and non-SSL case, and attempts
 * to hide the complexity setup from the user. This function
 * assumes that any certificate setup necessary has already
 * been done.
 *
 * If SSL or STARTTLS needs to be enabled, and the underlying
 * toolkit supports it, the following values are accepted for
 * secure:
 *
 * AP_LDAP_NONE: No encryption
 * AP_LDAP_SSL: SSL encryption (ldaps://)
 * AP_LDAP_STARTTLS: Force STARTTLS on ldap://
 */
MODLDAP_DECLARE(int) ap_ldap_init(apr_pool_t *pool,
                                  LDAP **ldap,
                                  const char *hostname,
                                  int portno,
                                  int secure,
                                  ap_ldap_err_t **result_err)
{

    ap_ldap_err_t *result = (ap_ldap_err_t *)apr_pcalloc(pool, sizeof(ap_ldap_err_t));
    *result_err = result;

#if AP_HAS_LDAPSSL_INIT
    *ldap = ldapssl_init(hostname, portno, 0);
#elif AP_HAS_LDAP_SSLINIT
    *ldap = ldap_sslinit((char *)hostname, portno, 0);
#else
    *ldap = ldap_init((char *)hostname, portno);
#endif
    if (*ldap != NULL) {
        return ap_ldap_set_option(pool, *ldap, AP_LDAP_OPT_TLS, &secure, result_err);
    }
    else {
        /* handle the error case */
        ap_ldap_err_t *result = (ap_ldap_err_t *)apr_pcalloc(pool, sizeof(ap_ldap_err_t));
        *result_err = result;

        result->reason = "APR LDAP: Unable to initialize the LDAP connection";
        result->rc = -1;
        return APR_EGENERAL;
    }

}


/**
 * APR LDAP info function
 *
 * This function returns a string describing the LDAP toolkit
 * currently in use. The string is placed inside result_err->reason.
 */
MODLDAP_DECLARE(int) ap_ldap_info(apr_pool_t *pool,
                                  ap_ldap_err_t **result_err)
{
    ap_ldap_err_t *result = (ap_ldap_err_t *)apr_pcalloc(pool, sizeof(ap_ldap_err_t));
    *result_err = result;

    result->reason = "APR LDAP: Built with "
                     LDAP_VENDOR_NAME
                     " LDAP SDK";
    return APR_SUCCESS;
    
}

#endif /* AP_HAS_LDAP */
