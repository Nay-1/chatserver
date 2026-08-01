# AGENTS.md

C++11（muduo）集群聊天服务器：nginx TCP 负载均衡 + 多 ChatServer 实例，通过 Redis 发布订阅做跨服务器消息转发。

## 构建

```bash
./autobuild.sh        # 等价于: mkdir -p build && rm -rf build/* && cd build && cmake .. && make -j 8
```

- 产物输出到仓库根目录 `bin/`（CMakeLists 设置了 `EXECUTABLE_OUTPUT_PATH=${PROJECT_SOURCE_DIR}/bin`），已有编译产物被 git 跟踪。
- 依赖系统库（非 vendored）：`muduo_net`、`muduo_base`、`mysqlclient`（链接路径硬编码 `/usr/lib64/mysql`）、`hiredis`、`pthread`。
- 头文件搜索路径是平铺的（include/server/db 等各自加进 include_directories），所以源码用 `#include "db.h"` 这种短路径。

## 运行（Docker 基础设施 + 宿主机应用层）

- 基础设施容器化：`docker compose up -d` 起 mysql:8.0（3306，自动导入 `chat.sql`）、redis:7（6379）、nginx:alpine（8000，TCP 轮询转发宿主机 6000/6001）。
- nginx 配置在 `nginx/nginx.conf`（stream 块，upstream 用 `host.docker.internal` 访问宿主机，依赖 compose 的 `extra_hosts: host-gateway`）。
- MySQL：库名 `chat`，连接参数从 `conf/chat.conf` 的 `[mysql]` 段读取（host/port/user/password/dbname，配置缺失时缺省 127.0.0.1 / 3306 / root / 123456 / chat，与 compose 环境变量一致），连接后 `set names gbk`（中文编码走 gbk）。配置读取模块在 `src/server/config.cpp`（INI 风格解析，分号注释）。
- Redis：从 `conf/chat.conf` 的 `[redis]` 段读取 host/port（缺省 127.0.0.1 / 6379）。`ChatService` 构造时连接，失败仅跳过回调注册，不致命；订阅线程独立阻塞接收。
- 启动：`./bin/ChatServer 0.0.0.0 6000`（实例1）、`./bin/ChatServer 0.0.0.0 6001`（实例2）；客户端：`./bin/ChatClient 127.0.0.1 8000`，终端菜单式，main 线程发、子线程收。
- 改了源码后：先 `./autobuild.sh` 重新编译，再重启 ChatServer 进程，无需动容器。

## 架构与约定

- 通信协议是 JSON over TCP，`msgid` 枚举定义在 `include/public.hpp`，服务端/客户端共享——改协议要两边同步。
- 业务核心 `ChatService` 单例（`src/server/chatservice.cpp`），构造时把 msgid 绑定到处理函数（`_msgHandlerMap`），新消息类型在这里注册。
- `src/server/chatserver.cpp` 的 `onMessage` 里 `json::parse` 有 try-catch 保护：收到非法数据（空连接、非 JSON）记录日志并断开连接，防止工作线程崩溃。
- `include/server/model/` 下各 model 类封装 MySQL 表操作（user、friend、group、offlinemessage）；`_userConnMap` 维护在线用户连接，受 `_connMutex` 保护。
- JSON 库是 nlohmann/json 单头文件，在 `thirdparty/json.hpp` 和 `test/testjson/json.hpp` 各有一份拷贝，改动需同步两处。
- 代码注释是中文，保持中文注释风格。

## 测试

- `test/` 不参与主构建（`src/CMakeLists.txt` 只加了 server/client）。
- `test/testmuduo/` 有独立 CMakeLists，需单独跑 cmake，产物 `server` 也输出到 `bin/`。
- `test/testjson/` 是手工 g++ 编译的二进制（无 CMakeLists），源码 `testjson.cpp` + json.hpp。
