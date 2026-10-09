# HTTP Server

The following configuration sets up an HTTP server for NanoMQ, including the server port, connection limit, authentication type, and JWT public key file if JWT authentication is used.

## Example Configuration

```hcl
http_server = {
  port = 8081               # HTTP server port
  ip_addr = "127.0.0.1"     # Bind to loopback by default
  limit_conn = 32           # Maximum number of outstanding requests NanoMQ can handle
  users = [                 # Basic authentication credentials
    { username = "admin", password = "replace-with-a-unique-password" }
  ]
  # username = "admin"      # Compatibility form, prefer users
  # password = "replace-with-a-unique-password" # Compatibility form, prefer users
  max_body = 65535          # Max size of HTTP body server can accept
  auth_type = "basic"       # Authentication type, "basic" or "jwt"
  jwt = {
    public.keyfile = "/etc/certs/jwt/jwtRS256.key.pub"   # Path to the JWT public key file, used if auth_type is "jwt"
  }
}
```

## Configuration Items

- `port`: Specifies the port on which the HTTP server will listen. Value range: 0 ~ 65535.
- `ip_addr`: Specifies the address on which the HTTP server listens. The default is `127.0.0.1`. An empty value is rejected; `0.0.0.0` explicitly enables access from other interfaces.
- `limit_conn`: Specifies the maximum number of outstanding requests that the server can handle at once. Value range: 1 ~ infinity.
- `users`: The list of Basic authentication credentials, each entry shaped
  `{ username = "...", password = "..." }`. Every credential present in the
  config is valid and any match grants access. This key is only supported by the
  HOCON config format. Entries with missing fields, empty strings or a `:` in
  the username are skipped with a warning; a duplicate username keeps the first
  entry. Basic authentication needs at least one valid `users` entry or a nonempty
  `username`/`password` pair; there are no built-in credentials, and without
  one the Admin API does not start. Configuration responses return every
  password as `null`; an update that sends `null` or omits a password keeps
  the current one.
- `username`: **Compatibility form**, still fully supported; prefer `users` for
  new configurations. It stays valid when present and forms a union with every
  `users` entry.
- `password`: **Compatibility form**, still fully supported. See `username`.
- `max_body`: Specifies the max size of HTTP body allowed by the HTTP server (Bytes).
- `auth_type`: Specifies the type of authentication used by the HTTP server. Values:
  - "basic"
  - "jwt": If "jwt" is to be used, make sure JWT dependencies have been built with the `-DENABLE_JWT=ON` option. For details, see [Build from Source Code](../installation/build-options.md)
- `jwt.public.keyfile`: Specifies the path to the public key file used for JWT authentication, used if `http_server.auth_type` is set to `jwt`. JWT mode requires JWT support and a loaded public key; otherwise the Admin API does not start.

The supplied default broker configuration leaves the Admin API disabled. Use a unique password when enabling it. Basic authentication sends credentials without encryption over HTTP; remote access should use a protected TLS endpoint. Listener address and port changes require a restart.
