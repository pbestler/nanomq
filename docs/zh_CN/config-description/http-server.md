# HTTP 服务器

本节将介绍如何配置 HTTP 服务器，包括服务器端口、连接限制、认证类型以及 JWT 认证。

## 配置示例

```hcl
http_server = {
  port = 8081               # HTTP 服务器端口
  ip_addr = "127.0.0.1"       # HTTP 服务器 Address
  limit_conn = 32           # NanoMQ 可以处理的最大未完成请求数
  users = [                 # Basic 认证凭据列表
    { username = "admin", password = "replace-with-a-unique-password" }
  ]
  # username = "admin"      # 兼容写法，新配置推荐 users
  # password = "replace-with-a-unique-password" # 兼容写法，新配置推荐 users
  max_body = 65535          # 最大 HTTP Body
  auth_type = "basic"       # 认证类型
  jwt = {
    public.keyfile = "/etc/certs/jwt/jwtRS256.key.pub"   # JWT公钥文件路径
  }
}
```

## 配置项

- `port`：HTTP 服务器的监听端口。取值范围：0 ~ 65535。
- `ip_addr`：监听地址，默认 `127.0.0.1`。空值会被拒绝；`0.0.0.0` 显式允许其他接口访问。
- `limit_conn`：服务器一次可以处理的最大未完成请求数量。值范围：1 ~  infinity。
- `users`：Basic 认证凭据列表，每个元素形如
  `{ username = "...", password = "..." }`。配置里出现的凭据都有效，任意一组
  匹配即可通过。该配置只在 HOCON 配置格式下支持。缺字段、空字符串、用户名中
  含 `:` 的条目会被跳过并打印告警，重复用户名保留第一条。Basic 认证至少需要一条
  有效的 `users` 条目或非空的 `username`/`password`，无内置凭据，否则管理 API
  不会启动。配置查询返回的密码均为 `null`；更新时传 `null` 或省略密码会保留当前
  密码。
- `username`：**兼容写法**，仍完全支持；新配置推荐用 `users`。只要写在配置里
  就仍然有效，并且与 `users` 里的每一条构成并集。
- `password`：**兼容写法**，仍完全支持；新配置推荐用 `users`。见 `username`。
- `max_body`：HTTP 服务器一次可以处理的最大 HTTP body.
- `auth_type`：HTTP 服务器使用的认证类型：
  - "basic"
  - "jwt"：如使用 `"jwt"`，请确保已启用 JWT 功能 （ `-DENABLE_JWT=ON`），具体步骤，见[源码编译安装](../installation/build-options.md)。
- `jwt.public.keyfile`：用于 JWT 认证的公钥文件的路径，仅在 `http_server.auth_type` 设为 `jwt` 时生效。

默认配置未启用管理 API。Basic 认证必须显式配置非空用户名和唯一密码，无内置凭据。JWT 认证需要启用 JWT 支持并成功加载公钥，否则管理 API 不会启动。监听地址或端口的更改需要重启。远程 Basic 认证应通过受保护的 TLS 端点访问。
