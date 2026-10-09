#include "conf_api.h"
#include "web_server.h"
#include "nng/supplemental/nanolib/nanolib.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

static void
test_listener_config(void)
{
	conf_http_server http;
	conf_http_server_init(&http, 8081);
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	http.username = nng_strdup("test");
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	http.password = nng_strdup("");
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	nng_strfree(http.password);
	http.password = nng_strdup("test-secret");
	assert(set_http_server_conf(&http) == 0);
	assert(strcmp(http.ip_addr, "127.0.0.1") == 0);
	nng_strfree(http.ip_addr);
	http.ip_addr = nng_strdup("127.0.0.2");
	assert(set_http_server_conf(&http) == 0);
	assert(strcmp(http.ip_addr, "127.0.0.2") == 0);
	nng_strfree(http.ip_addr);
	http.ip_addr = nng_strdup("");
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	http.auth_type = JWT;
#ifdef SUPP_JWT
	assert(set_http_server_conf(&http) == NNG_EINVAL);
#else
	assert(set_http_server_conf(&http) == NNG_ENOTSUP);
#endif
	http.auth_type = (auth_type_t) 99;
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	conf_http_server_destroy(&http);

	// A `users` entry alone is enough; an empty `users` list is not.
	conf_http_server_init(&http, 8081);
	http.auth_type = BASIC;
	cJSON *list = cJSON_Parse("[]");
	size_t count = 0;
	conf_http_user **users;
	users = conf_http_users_parse(list, &count);
	conf_http_server_set_users(&http, users, count);
	assert(set_http_server_conf(&http) == NNG_EINVAL);
	cJSON_Delete(list);
	list = cJSON_Parse("[{\"username\":\"ops\",\"password\":\"ops-secret\"}]");
	users = conf_http_users_parse(list, &count);
	conf_http_server_set_users(&http, users, count);
	assert(set_http_server_conf(&http) == 0);
	cJSON_Delete(list);
	conf_http_server_destroy(&http);
}

static void
test_config_responses(void)
{
	conf config = { 0 };
	conf_bridge_node node = { 0 };
	conf_bridge_node *nodes[] = { &node };
	conf_http_header header = { .key = "Authorization", .value = "header-secret" };
	conf_http_header *headers[] = { &header };
	char *users[] = { "test" }, *passwords[] = { "user-secret" };
	config.http_server.username = nng_strdup("test");
	config.http_server.password = nng_strdup("test-secret");
	config.http_server.auth_type = BASIC;
	assert(set_http_server_conf(&config.http_server) == 0);
	config.tls.key = "private-key-secret";
	config.tls.key_password = "key-password-secret";
	config.auths.count = 1;
	config.auths.usernames = users;
	config.auths.passwords = passwords;
	config.auth_http.auth_req.header_count = 1;
	config.auth_http.auth_req.headers = headers;
	node.name = "test-bridge";
	node.address = "mqtt-tcp://127.0.0.1:1883";
	node.password = "bridge-token-secret";
	node.tls = config.tls;
	config.bridge.count = 1;
	config.bridge.nodes = nodes;
	set_global_conf(&config);

	const char *paths[] = { "/api/v4/configuration",
		"/api/v4/configuration/bridge", "/api/v4/bridges/test-bridge" };
	for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
		http_msg req = { 0 }, res;
		put_http_msg(&req, NULL, "GET", paths[i],
		    "Basic dGVzdDp0ZXN0LXNlY3JldA==", NULL, 0);
		res = process_request(&req, &config.http_server, NULL);
		assert(res.status == 200);
		cJSON *json = cJSON_ParseWithLength(res.data, res.data_len);
		assert(json != NULL);
		char *text = cJSON_PrintUnformatted(json);
		assert(strstr(text, "secret") == NULL);
		cJSON_free(text);
		cJSON_Delete(json);
		destory_http_msg(&req);
		destory_http_msg(&res);
	}

	http_msg req = { 0 }, res;
	put_http_msg(&req, NULL, "GET", paths[0], NULL, NULL, 0);
	res = process_request(&req, &config.http_server, NULL);
	assert(res.status == 401);
	destory_http_msg(&res);
	config.http_server.auth_type = (auth_type_t) 99;
	res = process_request(&req, &config.http_server, NULL);
	assert(res.status == 401);
	destory_http_msg(&res);
	destory_http_msg(&req);
	conf_http_server_destroy(&config.http_server);
}

static void
test_secret_updates(void)
{
	conf_bridge_node node = { 0 };
	node.password = "old-token";
	node.tls.enable = true;
	node.tls.verify_peer = true;
	node.tls.sni = "server.test";
	node.tls.keyfile = "test-key.pem";
	node.tls.certfile = "test-cert.pem";
	node.tls.cafile = "test-ca.pem";
	node.tls.key_password = "old-pin";
	cJSON *json = cJSON_Parse("{\"password\":null,\"ssl\":{\"key_password\":null}}");
	assert(preserve_bridge_secrets(json, &node) == 0);
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(json, "password")),
	    "old-token") == 0);
	cJSON *ssl = cJSON_GetObjectItem(json, "ssl");
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(ssl, "key_password")),
	    "old-pin") == 0);
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(ssl, "keyfile")),
	    "test-key.pem") == 0);
	cJSON_Delete(json);
	json = cJSON_Parse("{\"password\":\"new-token\"}");
	assert(preserve_bridge_secrets(json, &node) == 0);
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(json, "password")),
	    "new-token") == 0);
	ssl = cJSON_GetObjectItem(json, "ssl");
	assert(cJSON_IsTrue(cJSON_GetObjectItem(ssl, "verify_peer")));
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(ssl, "sni")),
	    "server.test") == 0);
	cJSON_Delete(json);
	json = cJSON_Parse("{}");
	assert(preserve_bridge_secrets(json, &node) == 0);
	assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(json, "password")),
	    "old-token") == 0);
	cJSON_Delete(json);
	cJSON *request = cJSON_Parse("{\"test\":{\"server\":\"tls+nmq-tcp://server.test:8883\","
	    "\"password\":\"rotated-token\"}}");
	cJSON *connector = cJSON_GetObjectItem(request, "test");
	assert(preserve_bridge_secrets(connector, &node) == 0);
	conf_bridge_node updated = { 0 };
	conf_sqlite sqlite = { 0 };
	conf_bridge_node_init(&updated);
	conf_bridge_node_parse(&updated, &sqlite, connector);
	assert(updated.tls.enable && updated.tls.verify_peer);
	assert(strcmp(updated.tls.sni, "server.test") == 0);
	assert(strcmp(updated.tls.keyfile, "test-key.pem") == 0);
	assert(strcmp(updated.password, "rotated-token") == 0);
	conf_bridge_node_destroy(&updated);
	cJSON_Delete(request);
	json = cJSON_Parse("{\"password\":123}");
	assert(preserve_bridge_secrets(json, &node) == NNG_EINVAL);
	cJSON_Delete(json);

	conf_http_server http = { 0 };
	http.password = nng_strdup("old-admin-password");
	json = get_http_config(&http);
	set_http_config(json, NULL, &http);
	assert(strcmp(http.password, "old-admin-password") == 0);
	cJSON_Delete(json);
	json = cJSON_Parse("{\"password\":\"new-admin-password\"}");
	set_http_config(json, NULL, &http);
	assert(strcmp(http.password, "new-admin-password") == 0);
	cJSON_Delete(json);
	json = cJSON_Parse("{\"users\":[{\"username\":\"ops\","
	                   "\"password\":\"ops-password\"}]}");
	set_http_config(json, NULL, &http);
	cJSON_Delete(json);
	json = get_http_config(&http);
	cJSON *user = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "users"), 0);
	assert(cJSON_IsNull(cJSON_GetObjectItem(user, "password")));
	set_http_config(json, NULL, &http);
	cJSON_Delete(json);
	assert(conf_http_server_cred_check(&http, "ops:ops-password", 16));
	conf_http_server_destroy(&http);

	conf_auth auth = { 0 };
	assert(nng_mtx_alloc(&auth.mtx) == 0);
	cvector_push_back(auth.usernames, nng_strdup("test"));
	cvector_push_back(auth.passwords, nng_strdup("user-password"));
	auth.count = 1;
	json = get_auth_config(&auth);
	set_auth_config(json, NULL, &auth);
	assert(auth.count == 1 && strcmp(auth.passwords[0], "user-password") == 0);
	cJSON_Delete(json);
	nng_strfree(auth.usernames[0]);
	nng_strfree(auth.passwords[0]);
	cvector_free(auth.usernames);
	cvector_free(auth.passwords);
	nng_mtx_free(auth.mtx);

	conf_auth_http auth_http = { 0 };
	conf_http_header *header = nng_zalloc(sizeof(*header));
	header->key = nng_strdup("Authorization");
	header->value = nng_strdup("old-header-token");
	cvector_push_back(auth_http.auth_req.headers, header);
	auth_http.auth_req.header_count = 1;
	json = get_auth_http_config(&auth_http);
	set_auth_http_config(json, NULL, &auth_http);
	assert(strcmp(header->value, "old-header-token") == 0);
	cJSON_Delete(json);
	json = cJSON_Parse("{\"auth_req\":{\"headers\":{\"Other\":null,"
	    "\"Authorization\":\"new-header-token\",\"X-Test\":\"test-value\"}}}");
	set_auth_http_config(json, NULL, &auth_http);
	assert(auth_http.auth_req.header_count == 2);
	assert(strcmp(auth_http.auth_req.headers[1]->value, "test-value") == 0);
	assert(strcmp(header->value, "new-header-token") == 0);
	cJSON_Delete(json);
	for (size_t i = 0; i < auth_http.auth_req.header_count; i++) {
		header = auth_http.auth_req.headers[i];
		nng_strfree(header->key);
		nng_strfree(header->value);
		nng_free(header, sizeof(*header));
	}
	cvector_free(auth_http.auth_req.headers);
}

int
main(void)
{
	test_listener_config();
	test_config_responses();
	test_secret_updates();
	nng_fini();
	return 0;
}
