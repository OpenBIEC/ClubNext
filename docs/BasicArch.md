## **整体架构**
### 技术选型
1. **前端**：
   - **UI 框架**：Bootstrap（提供响应式样式）。
   - **客户端逻辑**：纯 JavaScript（使用 Fetch API 调用后端 REST API）。
   - **模板引擎（可选）**：无（HTML 页面为静态资源）。

2. **后端**：
   - **语言**：C++。
   - **HTTP 库**：`httplib`（轻量级 HTTP 服务）。
   - **数据存储**：SQLite/PostgreSQL（关系型数据库）+ JSON 文件（临时存储）。
   - **协议**：ActivityPub（JSON-LD 数据格式）。
   - **服务分发**：REST API。

3. **通信**：
   - **前后端通信**：REST API（JSON 格式）。
   - **后端对外通信**：基于 ActivityPub 协议的 S2S 数据传播。

---

## **系统架构图**
```
用户浏览器
    |
    V
静态前端服务器 (HTML/JS)
    |
    V
后端服务 (C++ httplib)
    |
    +--> 数据库 (SQLite/PostgreSQL)
    +--> 外部 ActivityPub 服务 (通过 HTTP)
```

---

## **前端设计**
前端负责展示页面，提供与后端 REST API 的交互。

### **文件结构**
```plaintext
frontend/
├── index.html         # 主页
├── login.html         # 登录页
├── register.html      # 注册页
├── profile.html       # 用户个人资料页
├── forum.html         # 论坛页
├── post.html          # 帖子详情页
├── private_message.html # 私信页
├── js/
│   ├── main.js        # 全局 JS 脚本
│   ├── api.js         # 封装的 API 调用
│   └── activitypub.js # ActivityPub 数据格式处理
├── css/
│   ├── style.css      # 自定义样式
└── assets/            # 静态资源 (图片、图标等)
```

### **主要页面功能**
1. **主页 (`index.html`)**
   - 显示论坛分类和热门帖子。
   - 用户登录状态检查，显示“登录”或“个人中心”。

2. **登录与注册页面**
   - 登录：通过 POST 请求向后端验证用户。
   - 注册：通过 POST 请求创建新用户。

3. **论坛页面**
   - 分类导航栏。
   - 帖子列表：通过 REST API 获取帖子数据。

4. **帖子详情页面**
   - 显示帖子内容及评论。
   - 提供评论框，用户可以发表评论。

5. **私信页面**
   - 消息列表：通过 REST API 获取消息记录。
   - 实时消息更新：使用轮询或 WebSocket。

### **示例：`main.js`**
```javascript
document.addEventListener("DOMContentLoaded", () => {
    const apiBase = "http://localhost:8080/api";

    // 获取热门帖子
    fetch(`${apiBase}/forum/posts`)
        .then(res => res.json())
        .then(data => {
            console.log("Posts:", data);
            // 将数据渲染到页面
        })
        .catch(err => console.error("Error fetching posts:", err));
});
```

---

## **后端设计**
后端使用 `httplib` 提供 REST API 服务，符合 ActivityPub 标准。

### **项目结构**
```plaintext
backend/
├── main.cpp           # 主程序入口
├── routes/            # 路由处理
│   ├── user.cpp       # 用户相关 API
│   ├── forum.cpp      # 论坛相关 API
│   └── message.cpp    # 私信相关 API
├── models/            # 数据库模型
│   ├── user.cpp
│   ├── post.cpp
│   └── message.cpp
├── utils/             # 工具类
│   ├── json.cpp       # JSON 解析
│   └── activitypub.cpp # ActivityPub 数据处理
└── db/
    └── database.db    # 数据库文件
```

### **核心代码**
#### 初始化 HTTP 服务器 (`main.cpp`)
```cpp
#include <httplib.h>
#include "routes/user.h"
#include "routes/forum.h"
#include "routes/message.h"

int main() {
    httplib::Server server;

    // 用户路由
    server.Post("/api/user/login", handle_user_login);
    server.Post("/api/user/register", handle_user_register);

    // 论坛路由
    server.Get("/api/forum/posts", handle_get_posts);
    server.Post("/api/forum/post", handle_create_post);

    // 私信路由
    server.Get("/api/message/inbox", handle_get_messages);
    server.Post("/api/message/send", handle_send_message);

    // 启动服务器
    server.listen("0.0.0.0", 8080);
    return 0;
}
```

#### 用户登录接口示例 (`user.cpp`)
```cpp
#include <httplib.h>
#include <json/json.h>

void handle_user_login(const httplib::Request &req, httplib::Response &res) {
    Json::Reader reader;
    Json::Value body;
    if (!reader.parse(req.body, body)) {
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string username = body["username"].asString();
    std::string password = body["password"].asString();

    // 验证用户（示例代码）
    if (username == "student" && password == "password") {
        Json::Value response;
        response["message"] = "Login successful";
        res.set_content(response.toStyledString(), "application/json");
    } else {
        res.status = 401;
        res.set_content("{\"error\":\"Invalid credentials\"}", "application/json");
    }
}
```

### **数据库设计**
后端数据库使用 SQLite 或 PostgreSQL。数据库表设计参考之前的数据库方案。

---

## **前后端交互流程**

1. **用户登录**：
   - 前端：通过 `POST /api/user/login` 提交用户名和密码。
   - 后端：验证用户并返回登录状态。

2. **获取帖子列表**：
   - 前端：通过 `GET /api/forum/posts` 获取数据。
   - 后端：查询数据库，返回 JSON 格式的帖子数据。

3. **发表评论**：
   - 前端：通过 `POST /api/forum/post/<id>/comment` 提交评论。
   - 后端：解析评论数据并更新数据库。

4. **发送私信**：
   - 前端：通过 `POST /api/message/send` 提交消息。
   - 后端：验证消息并分发到目标用户。
