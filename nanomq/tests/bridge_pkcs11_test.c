//
// Copyright 2026 Peter Bestler <peter.bestler@liebherr.com>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#include <assert.h>

#include "include/bridge.h"
#include "include/mqtt_api.h"
#include "nng/supplemental/nanolib/nanolib.h"

#ifdef NNG_SUPP_TLS
static const char *bridge_client_cert_uri =
    "pkcs11:token=NanoMQ;object=bridge-cert;type=cert";
static const char *bridge_client_key_uri =
    "pkcs11:token=NanoMQ;object=bridge-key;type=private";
static const char *bridge_ca_uri =
    "pkcs11:token=NanoMQ;object=bridge-ca;type=cert";
static const char *bridge_test_ca_pem =
    "-----BEGIN CERTIFICATE-----\nnot-a-pkcs11-uri\n";

static void
test_bridge_pkcs11_accepts_pem_ca(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = nng_strdup(bridge_client_cert_uri);
	tls.key  = nng_strdup(bridge_client_key_uri);
	tls.ca   = nng_strdup(bridge_test_ca_pem);

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(tls.ca != NULL);
	assert(bridge_tls_validate_pkcs11(&tls) == 0);

	conf_tls_destroy(&tls);
}

static void
test_bridge_pkcs11_accepts_ca_uri_without_client_credentials(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.ca = nng_strdup(bridge_ca_uri);

	assert(tls.ca != NULL);
	assert(bridge_tls_validate_pkcs11(&tls) == 0);

	conf_tls_destroy(&tls);
}

static void
test_bridge_pkcs11_rejects_mixed_cert_key_sources(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = nng_strdup(bridge_client_cert_uri);
	tls.key =
	    nng_strdup("-----BEGIN PRIVATE KEY-----\nnot-a-pkcs11-uri\n");

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(bridge_tls_validate_pkcs11(&tls) == NNG_EINVAL);

	conf_tls_destroy(&tls);
}

static void
test_listener_pkcs11_accepts_complete_uri_set(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = nng_strdup(bridge_client_cert_uri);
	tls.key  = nng_strdup(bridge_client_key_uri);
	tls.ca   = nng_strdup(bridge_ca_uri);

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(tls.ca != NULL);
	assert(nano_tls_validate_listener_pkcs11(&tls) == 0);

	conf_tls_destroy(&tls);
}

static void
test_listener_pkcs11_rejects_pem_ca(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = nng_strdup(bridge_client_cert_uri);
	tls.key  = nng_strdup(bridge_client_key_uri);
	tls.ca   = nng_strdup(bridge_test_ca_pem);

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(tls.ca != NULL);
	assert(nano_tls_validate_listener_pkcs11(&tls) == NNG_EINVAL);

	conf_tls_destroy(&tls);
}

static void
test_listener_pkcs11_rejects_ca_uri_without_identity(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.ca = nng_strdup(bridge_ca_uri);

	assert(tls.ca != NULL);
	assert(nano_tls_validate_listener_pkcs11(&tls) == NNG_EINVAL);

	conf_tls_destroy(&tls);
}
#endif

int
main(void)
{
#ifdef NNG_SUPP_TLS
	test_bridge_pkcs11_accepts_pem_ca();
	test_bridge_pkcs11_accepts_ca_uri_without_client_credentials();
	test_bridge_pkcs11_rejects_mixed_cert_key_sources();
	test_listener_pkcs11_accepts_complete_uri_set();
	test_listener_pkcs11_rejects_pem_ca();
	test_listener_pkcs11_rejects_ca_uri_without_identity();
#endif
	return (0);
}
