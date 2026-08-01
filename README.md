# chatserver
基于nginx TCP负载均衡的集群聊天服务器

# 编译方式
mkdir build && cd build
cmake .. && make

或直接执行：
./autobuild.sh

# 部署方式（Docker 基础设施 + 宿主机应用层）

nginx、MySQL、Redis 用 docker compose 容器化，ChatServer/ChatClient 在宿主机直接运行编译产物。

## 1. 启动基础设施容器

```bash
docker compose up -d
```

启动 3 个容器：
- `mysql:8.0`：端口 3306，首次启动自动导入 `chat.sql`（root 密码 123456，与 db.cpp 硬编码一致），数据持久化到命名卷
- `redis:7`：端口 6379，跨 ChatServer 消息转发通道
- `nginx:alpine`：端口 8000，TCP 负载均衡，轮询转发到宿主机 6000/6001 端口（通过 host-gateway 访问宿主机）

## 2. 启动应用层（宿主机）

```bash
# 等 mysql 就绪（首次导入 chat.sql 需 30s 以上，docker compose ps 查看 healthy）
./bin/ChatServer 0.0.0.0 6000   # 实例1
./bin/ChatServer 0.0.0.0 6001   # 实例2
```

## 3. 启动客户端（宿主机）

```bash
./bin/ChatClient 127.0.0.1 8000
```

客户端通过 nginx 8000 入口接入，多开几个即可验证跨服务器消息转发。

## 注意事项

- 改了 C++ 源码后：先 `./autobuild.sh` 重新编译，再重启 ChatServer 进程，无需动容器
- 连接地址无需改动：db.cpp/redis.cpp 硬编码 127.0.0.1，恰好连通映射到宿主的容器端口
- `docker compose stop` 停容器不影响 ChatServer 进程；ChatServer 用 Ctrl+C（SIGINT）停止会重置用户 online 状态
