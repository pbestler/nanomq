# Listener 

The Listener configuration allows you to specify settings for different types of connections that your NanoMQ broker can accept. There are four types of listeners: TCP Listener, SSL Listener, WebSocket Listener, and Secure WebSocket Listener.

## MQTT/TCP Listener - 1883

### **Example Configuration**

```hcl
listeners.tcp {
  bind = "0.0.0.0:1883"     # The listener binds to all network interfaces on port 1883
}

listeners.tcp.listener_1 {
  bind = "0.0.0.0:1884"     # The listener binds to all network interfaces on port 1884
}

listeners.tcp.listener_2 {
  bind = "0.0.0.0:1885"     # The listener binds to all network interfaces on port 1885
}
```
NanoMQ support multi-listeners now. 

### **Configuration Items**

- `bind`: Specifies the IP address and port that the TCP listener should bind to. The value should be in the format `<ip:port>`.

## MQTT/SSL Listener - 8883

### **Example Configuration**

```hcl
listeners.ssl {
  bind = "0.0.0.0:8883"              # Bind to all network interfaces on port 8883
  # key_password = <yourpass>        # Private key password or PKCS#11 PIN
  keyfile = "/etc/certs/key.pem"     # Key file path or PKCS#11 URI
  certfile = "/etc/certs/cert.pem"   # Certificate path or PKCS#11 URI
  cacertfile = "/etc/certs/cacert.pem" # CA certificate file path
  verify_peer = false					  		 # If NanoMQ requests a certificate from the client 	
  fail_if_no_peer_cert = false			 # If to reject connection if no certificate is provided
}
```

### **Configuration Items**

- `bind`: Specifies the IP address and port that the SSL listener should bind to.
- `key_password`: Specifies the password for an encrypted private key. For a
  PKCS#11 key URI, it is passed to the provider as the token PIN.
- `keyfile`: Specifies a private key file path or PKCS#11 URI.
- `certfile`: Specifies a certificate file path or PKCS#11 URI.
- `cacertfile`: Specifies a PEM-encoded CA file path or PKCS#11 URI.
- `verify_peer`: Specifies whether the server requests a certificate from the client, optional value: 
  - `true`: verify_peer
  - `false `: verify_none
- `fail_if_no_peer_cert`: Specifies whether to deny the connection if no certificate is provided, valid only when `verify_peer` is set to true, optional values: 
  - `true`: Rejects the connection if the client sends an empty certificate.
  - `false`: Rejects the connection only when the client sends an invalid certificate.

### **PKCS#11 Example**

PKCS#11 URIs require the OpenSSL TLS engine and OpenSSL 3 or newer. Build with
`-DNNG_ENABLE_TLS=ON -DNNG_TLS_ENGINE=open`. Adding
`-DNNG_REQUIRE_PKCS11_PROVIDER=ON` makes CMake reject an older OpenSSL version.
At runtime, the OpenSSL provider named `pkcs11` must be installed and loadable.

Listener PKCS#11 mode is enabled when any credential is a PKCS#11 URI. In that
mode, `keyfile` and `certfile` must both be PKCS#11 URIs. If `cacertfile` is
configured, it must also be a PKCS#11 URI.

```hcl
listeners.ssl {
  bind = "0.0.0.0:8883"
  keyfile = "pkcs11:token=NanoMQ;object=broker-key;type=private"
  certfile = "pkcs11:token=NanoMQ;object=broker-cert;type=cert"
  cacertfile = "pkcs11:token=NanoMQ;object=broker-ca;type=cert"
}
```

## MQTT/WebSocket Listener - 8083

### **Example Configuration**

```hcl
listeners.ws {
  bind = "0.0.0.0:8083/mqtt"			# Bind to all network interfaces on port 8083
}
```

### **Configuration Items**

- `bind`: Specifies the IP address and port that the WebSocket listener should bind to.

## MQTT/Secure WebSocket Listener - 8084

### **Example Configuration**

```hcl
listeners.wss {
  bind = "0.0.0.0:8084"           	# Bind to all network interfaces on port 8084
}
```

### **Configuration Items**

- `bind`: Specifies the IP address and port that the Secure WebSocket listener should bind to.

::: tip

The secure WebSocket listener utilizes the same `keyfile`, `certfile`, and `cacertfile` as the SSL listener. Therefore, if these certificate-related items have already been set for the SSL listener, there is no need to configure them again for the secure WebSocket listener. However, if no SSL listener has been configured, you will need to set the following configurations for the secure WebSocket listener:

- `keyfile`
- `certfile`
- `cacertfile`

:::
