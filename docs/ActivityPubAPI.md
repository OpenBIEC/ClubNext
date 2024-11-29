# ActivityPub论坛API

## **ActivityPub 基础概念**

- **Actor**: 用户或服务的表示，通常对应用户 (`Person`)。
- **Object**: 操作的目标，如帖子、评论、图片等。
- **Activity**: 动作的表达，如 `Create`、`Like`、`Follow`。

---

## **用户相关 API 的 ActivityPub 对应**

1. **用户注册**
   - **不直接适用 ActivityPub**，注册流程是内部逻辑，与 ActivityPub 无直接交互。

2. **用户登录**
   - **不直接适用 ActivityPub**，登录为内部功能，与分布式协议无关。

3. **获取当前用户信息**
   - ActivityPub `Actor` 对象。
   - **GET** `/api/activitypub/actor/{username}`
   - 返回 ActivityPub Actor 数据：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Person",
       "id": "https://example.com/users/{username}",
       "name": "{username}",
       "icon": {
         "type": "Image",
         "url": "https://example.com/avatar/{username}"
       },
       "summary": "{bio}",
       "following": "https://example.com/users/{username}/following",
       "followers": "https://example.com/users/{username}/followers",
       "inbox": "https://example.com/users/{username}/inbox",
       "outbox": "https://example.com/users/{username}/outbox"
     }
     ```

4. **获取陌生用户的基础资料**
   - 与当前用户信息相同，返回 ActivityPub `Actor`。

5. **上传/更新头像**
   - 不直接适用 ActivityPub，头像更改通常在 `Actor` 的 `icon` 字段更新后传播给订阅者。

---

## **帖子相关 API 的 ActivityPub 对应**

1. **获取帖子详情**
   - ActivityPub `Object` 表示帖子的内容。
   - **GET** `/api/activitypub/object/{post_id}`
   - 返回帖子的 ActivityPub 格式：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Note",
       "id": "https://example.com/post/{post_id}",
       "attributedTo": "https://example.com/users/{author_id}",
       "content": "{content}",
       "attachment": [
         {
           "type": "Image",
           "url": "https://example.com/media/{media_id}"
         }
       ],
       "to": ["https://www.w3.org/ns/activitystreams#Public"],
       "likes": "https://example.com/post/{post_id}/likes",
       "replies": "https://example.com/post/{post_id}/replies"
     }
     ```

2. **发布新帖子**
   - ActivityPub `Create` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Create",
       "actor": "https://example.com/users/{username}",
       "object": {
         "type": "Note",
         "content": "{content}",
         "to": ["https://www.w3.org/ns/activitystreams#Public"]
       }
     }
     ```

3. **上传帖子媒体**
   - 可用 ActivityPub `Attachment` 表示，但实际上传过程为内部逻辑。
   - 媒体上传完成后，可通过更新帖子的 `attachment` 字段将链接传播到订阅者。

4. **获取推荐帖子**
   - 不直接适用 ActivityPub，推荐逻辑为内部功能。

---

## **私信相关 API 的 ActivityPub 对应**

1. **获取私信列表**
   - 使用 ActivityPub `Inbox`。
   - **GET** `/api/activitypub/inbox`
   - 返回用户私信（非公共）：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "OrderedCollection",
       "id": "https://example.com/users/{username}/inbox",
       "totalItems": 10,
       "orderedItems": [
         {
           "type": "Note",
           "content": "{message_content}",
           "attributedTo": "https://example.com/users/{sender_id}",
           "to": ["https://example.com/users/{username}"]
         }
       ]
     }
     ```

2. **发送私信**
   - ActivityPub `Create` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Create",
       "actor": "https://example.com/users/{username}",
       "object": {
         "type": "Note",
         "content": "{message_content}",
         "to": ["https://example.com/users/{recipient_id}"]
       }
     }
     ```

---

## **社交功能 API 的 ActivityPub 对应**

1. **关注用户**
   - ActivityPub `Follow` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Follow",
       "actor": "https://example.com/users/{username}",
       "object": "https://example.com/users/{target_user_id}"
     }
     ```

2. **取消关注用户**
   - ActivityPub `Undo` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Undo",
       "actor": "https://example.com/users/{username}",
       "object": {
         "type": "Follow",
         "actor": "https://example.com/users/{username}",
         "object": "https://example.com/users/{target_user_id}"
       }
     }
     ```

3. **点赞帖子**
   - ActivityPub `Like` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Like",
       "actor": "https://example.com/users/{username}",
       "object": "https://example.com/post/{post_id}"
     }
     ```

4. **取消点赞**
   - ActivityPub `Undo` Activity。
   - **POST** `/api/activitypub/outbox`
   - 请求体：

     ```json
     {
       "@context": "https://www.w3.org/ns/activitystreams",
       "type": "Undo",
       "actor": "https://example.com/users/{username}",
       "object": {
         "type": "Like",
         "actor": "https://example.com/users/{username}",
         "object": "https://example.com/post/{post_id}"
       }
     }
     ```
