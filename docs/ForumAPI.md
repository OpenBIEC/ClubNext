# 基础论坛API

## 用户相关 API

### 1. **用户注册**

- **方法**: `POST`
- **路径**: `/api/user/register`
- **描述**: 注册新账号。
- **请求体**:

  ```json
  {
    "username": "string",
    "password": "string",
    "bio": "string"
  }
  ```

- **响应**:
  - 成功：

    ```json
    {
      "message": "User registered successfully"
    }
    ```

  - 错误：

    ```json
    {
      "error": "Username already exists"
    }
    ```

---

### 2. **用户登录**

- **方法**: `POST`
- **路径**: `/api/user/login`
- **描述**: 登录并获取会话 cookie。
- **请求体**:

  ```json
  {
    "username": "string",
    "password": "string"
  }
  ```

- **响应**:
  - 成功：

    ```json
    {
      "message": "Login successful"
    }
    ```

  - 错误：

    ```json

    {
      "error": "Invalid credentials"
    }
    ```

---

### 3. **获取当前用户信息**

- **方法**: `GET`
- **路径**: `/api/user/profile`
- **描述**: 获取当前登录用户的信息。
- **响应**:

  ```json
  {
    "username": "string",
    "bio": "string",
    "avatar": "url",
    "followers": "int",
    "followings": "int",
    "joined_at": "datetime"
  }
  ```

---

### 4. **上传/更新头像**

- **方法**: `POST`
- **路径**: `/api/user/avatar`
- **描述**: 上传或更新用户头像。
- **请求体**:
  - `Content-Type`: `multipart/form-data`
  - 文件字段名: `avatar`
- **响应**:

  ```json
  {
    "message": "Avatar updated successfully"
  }
  ```

---

### 5. **获取陌生用户的基础资料**

- **方法**: `GET`
- **路径**: `/api/user/{user_id}/profile`
- **描述**: 根据用户 ID 获取基础资料。
- **响应**:

  ```json
  {
    "username": "string",
    "bio": "string",
    "avatar": "url",
    "followers": "int",
    "followings": "int",
    "joined_at": "datetime"
  }
  ```

---

## 帖子相关 API

### 1. **获取帖子详情**

- **方法**: `GET`
- **路径**: `/api/forum/post/{post_id}`
- **描述**: 获取指定帖子的详情。
- **响应**:

  ```json
  {
    "id": "int",
    "author_id": "int",
    "content": "string",
    "media": ["url"],
    "like_count": "int",
    "liked_by_user": "boolean",
    "comment_count": "int"
  }
  ```

---

### 2. **发布新帖子**

- **方法**: `POST`
- **路径**: `/api/forum/post`
- **描述**: 创建新帖子。
- **请求体**:

  ```json
  {
    "title": "string",
    "content": "string"
  }
  ```

- **响应**:

  ```json
  {
    "message": "Post created successfully",
    "post_id": "int"
  }
  ```

---

### 3. **上传帖子媒体**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}/media`
- **描述**: 上传指定帖子的媒体文件。
- **请求体**:
  - `Content-Type`: `multipart/form-data`
  - 文件字段名: `media`
- **响应**:

  ```json
  {
    "message": "Media uploaded successfully"
  }
  ```

---

### 4. **获取推荐帖子**

- **方法**: `GET`
- **路径**: `/api/forum/recommend`
- **描述**: 返回最多 10 个推荐帖子的 ID。
- **响应**:

  ```json
  {
    "recommended_posts": [
      { "id": "int" },
      { "id": "int" },
      ...
    ]
  }
  ```

---

## 私信相关 API

### 1. **获取私信列表**

- **方法**: `GET`
- **路径**: `/api/message/inbox`
- **描述**: 获取当前用户的私信列表。
- **响应**:

  ```json
  {
    "messages": [
      {
        "sender_id": "int",
        "content": "string",
        "timestamp": "datetime"
      },
      ...
    ]
  }
  ```

---

### 2. **发送私信**

- **方法**: `POST`
- **路径**: `/api/message/send`
- **描述**: 向指定用户发送私信。
- **请求体**:

  ```json
  {
    "recipient_id": "int",
    "content": "string"
  }
  ```

- **响应**:

  ```json
  {
    "message": "Message sent successfully"
  }
  ```

---

## 社交功能 API

### 1. **关注用户**

- **方法**: `POST`
- **路径**: `/api/user/{user_id}/follow`
- **描述**: 关注指定用户。
- **响应**:

  ```json
  {
    "message": "Followed successfully"
  }
  ```

---

### 2. **取消关注用户**

- **方法**: `DELETE`
- **路径**: `/api/user/{user_id}/unfollow`
- **描述**: 取消关注指定用户。
- **响应**:

  ```json
  {
    "message": "Unfollowed successfully"
  }
  ```

---

### 3. **点赞帖子**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}/like`
- **描述**: 点赞帖子。
- **响应**:

  ```json
  {
    "message": "Post liked successfully"
  }
  ```

---

### 4. **取消点赞**

- **方法**: `DELETE`
- **路径**: `/api/forum/post/{post_id}/like`
- **描述**: 取消点赞帖子。
- **响应**:

  ```json
  {
    "message": "Post unliked successfully"
  }
  ```
  