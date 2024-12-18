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
    "email":"string",
    "password": "string"
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
    "password": "string",
    "remember_me": "any" // Optional
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

### 4. **更改用户个人资料 API**

**方法**: `POST`  
**路径**: `/api/user/profile/update`  
**描述**: 更新当前登录用户的个人资料。  

**请求体**:

```json
{
  "bio": "string",
  "avatar_url": "string"
}
```

**响应**:

- **成功**:

  ```json
  {
    "message": "Profile updated successfully",
    "token": "<TOKEN>"
  }
  ```

---

### 5. **上传/更新头像**

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

### 6. **获取陌生用户的基础资料**

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

### 7. **发送验证码**

- **URL**: `/api/user/verify`
- **方法**: `GET`
- **描述**: 发送验证码到指定的用户邮箱。

- **请求参数**

| 参数名   | 类型     | 是否必填 | 描述             |
|----------|----------|----------|------------------|
| `username`  | `string` | 必填     | 用户名。 |
| `email`  | `string` | 必填     | 用户接收验证码的邮箱地址。 |

- **响应**

```json
{
    "message": "Verification code sent successfully"
}
```

---

### 8. **验证验证码**

- **URL**: `/api/user/validate`
- **方法**: `GET`
- **描述**: 验证用户输入的验证码是否正确。

- **请求参数**

| 参数名   | 类型     | 是否必填 | 描述                  |
|----------|----------|----------|-----------------------|
| `username`  | `string` | 必填     | 用户名。 |
| `code`   | `string` | 必填     | 用户输入的验证码。      |

- **响应**

```json
{
    "message": "Verification successful"
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
    "author": "string",
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

### 3. **修改帖子**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}`
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

### 4. **上传帖子媒体**

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

### 5. **获取推荐帖子**

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

## 标签相关 API

### 1. **获取标签列表**

- **方法**: `GET`
- **路径**: `/api/tags`
- **描述**: 获取所有可用标签的列表。
- **响应**:

  ```json
  {
    "tags": [
      {
        "id": "int",
        "name": "string"
      },
      ...
    ]
  }
  ```

---

### 2. **获取帖子的标签**

- **方法**: `GET`
- **路径**: `/api/forum/post/{post_id}/tags`
- **描述**: 获取指定帖子的标签列表。
- **响应**:

  ```json
  {
    "post_id": "int",
    "tags": [
      {
        "id": "int",
        "name": "string"
      },
      ...
    ]
  }
  ```

---

### 3. **给帖子添加标签**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}/tags`
- **描述**: 为指定帖子添加标签（需登录）。
- **请求体**:

  ```json
  {
    "tags": [
      { "id": "int" },
      { "id": "int" },
      ...
    ]
  }
  ```

- **响应**:
  - 成功：

    ```json
    {
      "message": "Tags added successfully"
    }
    ```

  - 错误：

    ```json
    {
      "error": "Tag does not exist"
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
        "sender_id": "string",
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
    "recipient_id": "string",
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

---

## **评论功能 API**

### 1. **获取帖子评论列表**

- **方法**: `GET`
- **路径**: `/api/forum/post/{post_id}/comments`
- **描述**: 获取指定帖子的评论列表。
- **响应**:

  ```json
  {
    "post_id": "int",
    "comments": [
      {
        "comment_id": "int",
        "author": "string",
        "content": "string",
        "timestamp": "datetime"
      },
      ...
    ]
  }
  ```

---

### 2. **发布评论**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}/comments`
- **描述**: 在指定帖子下发布评论。
- **请求体**:

  ```json
  {
    "author": "string",
    "content": "string"
  }
  ```

- **响应**:

  ```json
  {
    "message": "Comment posted successfully",
    "comment_id": "int"
  }
  ```

---

### 3. **编辑评论**

- **方法**: `PUT`
- **路径**: `/api/forum/post/{post_id}/comment/{comment_id}`
- **描述**: 编辑已发布的评论。
- **请求体**:

  ```json
  {
    "content": "string"
  }
  ```

- **响应**:

  ```json
  {
    "message": "Comment updated successfully"
  }
  ```

---

### 4. **删除评论**

- **方法**: `DELETE`
- **路径**: `/api/forum/post/{post_id}/comment/{comment_id}`
- **描述**: 删除指定评论。
- **响应**:

  ```json
  {
    "message": "Comment deleted successfully"
  }
  ```

---

### 5. **上传帖子媒体**

- **方法**: `POST`
- **路径**: `/api/forum/post/{post_id}/comment/{comment_id}/media`
- **描述**: 上传指定帖子的指定评论的媒体文件。
- **请求体**:
  - `Content-Type`: `multipart/form-data`
  - 文件字段名: `media`
- **响应**:

  ```json
  {
    "message": "Media uploaded successfully"
  }
  ```
