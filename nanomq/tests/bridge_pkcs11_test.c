#include <assert.h>
#include <string.h>

#include "include/bridge.h"
#include "nng/supplemental/nanolib/file.h"
#include "nng/supplemental/nanolib/nanolib.h"

#ifdef NNG_SUPP_TLS
static const char *bridge_client_cert_uri =
    "pkcs11:token=NanoMQ;object=bridge-cert;type=cert";
static const char *bridge_client_key_uri =
    "pkcs11:token=NanoMQ;object=bridge-key;type=private";
static const char *bridge_ca_uri =
    "pkcs11:token=NanoMQ;object=bridge-ca;type=cert";
static const char *bridge_test_ca_pem =
    "../../../etc/certs/cacert.pem";

static void
test_bridge_pkcs11_accepts_pem_ca(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = strdup(bridge_client_cert_uri);
	tls.key  = strdup(bridge_client_key_uri);

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(file_load_data(bridge_test_ca_pem, (void **) &tls.ca) != 0);
	assert(bridge_tls_validate_pkcs11(&tls) == 0);

	conf_tls_destroy(&tls);
}

static void
test_bridge_pkcs11_accepts_ca_uri_without_client_credentials(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.ca = strdup(bridge_ca_uri);

	assert(tls.ca != NULL);
	assert(bridge_tls_validate_pkcs11(&tls) == 0);

	conf_tls_destroy(&tls);
}

static void
test_bridge_pkcs11_rejects_mixed_cert_key_sources(void)
{
	conf_tls tls;

	conf_tls_init(&tls);
	tls.cert = strdup(bridge_client_cert_uri);
	tls.key  = strdup("-----BEGIN PRIVATE KEY-----\nnot-a-pkcs11-uri\n");

	assert(tls.cert != NULL);
	assert(tls.key != NULL);
	assert(bridge_tls_validate_pkcs11(&tls) == NNG_EINVAL);

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
#endif
	return 0;
}
