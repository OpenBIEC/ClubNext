# API Documentation

## Overview

This document provides a comprehensive overview of the available API endpoints grouped by functionality.

---

## User Management

### 1. **User Registration**

- **Method**: `POST`
- **Endpoint**: `/api/user/register`
- **Description**: Register a new user account.
- **Request Body**:

  ```json
  {
    "username": "string",
    "email": "string",
    "password": "string"
  }
  ```

- **Responses**:
  - Success:

    ```json
    {
      "message": "User registered successfully"
    }
    ```

  - Error:

    ```json
    {
      "error": "Username already exists"
    }
    ```

### 2. **User Login**

- **Method**: `POST`
- **Endpoint**: `/api/user/login`
- **Description**: Log in and retrieve session cookies.
- **Request Body**:

  ```json
  {
    "username": "string",
    "password": "string",
    "remember_me": "boolean" // Optional
  }
  ```

- **Responses**:
  - Success:

    ```json
    {
      "message": "Login successful"
    }
    ```

  - Error:

    ```json
    {
      "error": "Invalid credentials"
    }
    ```

### 3. **Get Current User Profile**

- **Method**: `GET`
- **Endpoint**: `/api/user/profile`
- **Description**: Retrieve the profile of the currently logged-in user.
- **Response**:

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

### 4. **Update User Profile**

- **Method**: `POST`
- **Endpoint**: `/api/user/profile/update`
- **Description**: Update the profile of the currently logged-in user.
- **Request Body**:

  ```json
  {
    "bio": "string",
    "avatar_url": "string"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Profile updated successfully"
  }
  ```

### 5. **Upload/Update Avatar**

- **Method**: `POST`
- **Endpoint**: `/api/user/avatar`
- **Description**: Upload or update the user's avatar.
- **Request Body**:
  - `Content-Type`: `multipart/form-data`
  - Field Name: `avatar`
- **Response**:

  ```json
  {
    "message": "Avatar updated successfully"
  }
  ```

### 6. **Send Verification Code**

- **Method**: `GET`
- **Endpoint**: `/api/user/verify`
- **Description**: Send a verification code to the user's email.
- **Query Parameters**:
  - `username`: `string` (Required)
  - `email`: `string` (Required)
- **Response**:

  ```json
  {
    "message": "Verification code sent successfully"
  }
  ```

### 7. **Validate Verification Code**

- **Method**: `GET`
- **Endpoint**: `/api/user/validate`
- **Description**: Validate the provided verification code.
- **Query Parameters**:
  - `username`: `string` (Required)
  - `code`: `string` (Required)
- **Response**:

  ```json
  {
    "message": "Verification successful"
  }
  ```

---

## Forum Post Management

### 1. **Get Post Details**

- **Method**: `GET`
- **Endpoint**: `/api/forum/post/{post_id}`
- **Description**: Retrieve details of a specific post.
- **Response**:

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

### 2. **Create New Post**

- **Method**: `POST`
- **Endpoint**: `/api/forum/post`
- **Description**: Create a new forum post.
- **Request Body**:

  ```json
  {
    "title": "string",
    "content": "string"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Post created successfully",
    "post_id": "int"
  }
  ```

### 3. **Upload Post Media**

- **Method**: `POST`
- **Endpoint**: `/api/forum/post/{post_id}/media`
- **Description**: Upload media files for a specific post.
- **Request Body**:
  - `Content-Type`: `multipart/form-data`
  - Field Name: `media`
- **Response**:

  ```json
  {
    "message": "Media uploaded successfully"
  }
  ```

### 4. **Get Recommended Posts**

- **Method**: `GET`
- **Endpoint**: `/api/forum/recommend`
- **Description**: Retrieve a list of recommended posts.
- **Response**:

  ```json
  {
    "recommended_posts": [
      { "id": "int" },
      { "id": "int" }
    ]
  }
  ```

---

## Comments Management

### 1. **Get Post Comments**

- **Method**: `GET`
- **Endpoint**: `/api/forum/post/{post_id}/comments`
- **Description**: Retrieve the list of comments for a specific post.
- **Response**:

  ```json
  {
    "post_id": "int",
    "comments": [
      {
        "comment_id": "int",
        "author": "string",
        "content": "string",
        "timestamp": "datetime"
      }
    ]
  }
  ```

### 2. **Post a Comment**

- **Method**: `POST`
- **Endpoint**: `/api/forum/post/{post_id}/comments`
- **Description**: Add a comment to a specific post.
- **Request Body**:

  ```json
  {
    "author": "string",
    "content": "string"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Comment posted successfully",
    "comment_id": "int"
  }
  ```

### 3. **Edit a Comment**

- **Method**: `PUT`
- **Endpoint**: `/api/forum/post/{post_id}/comment/{comment_id}`
- **Description**: Edit an existing comment.
- **Request Body**:

  ```json
  {
    "content": "string"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Comment updated successfully"
  }
  ```

### 4. **Delete a Comment**

- **Method**: `DELETE`
- **Endpoint**: `/api/forum/post/{post_id}/comment/{comment_id}`
- **Description**: Delete a specific comment.
- **Response**:

  ```json
  {
    "message": "Comment deleted successfully"
  }
  ```

### 5. **Upload Comment Media**

- **Method**: `POST`
- **Endpoint**: `/api/forum/post/{post_id}/comment/{comment_id}/media`
- **Description**: Upload media files for a specific comment.
- **Request Body**:
  - `Content-Type`: `multipart/form-data`
  - Field Name: `media`
- **Response**:

  ```json
  {
    "message": "Media uploaded successfully"
  }
  ```

---

## Messaging

### 1. **Get Inbox**

- **Method**: `GET`
- **Endpoint**: `/api/message/inbox`
- **Description**: Retrieve the inbox messages of the current user.
- **Response**:

  ```json
  {
    "messages": [
      {
        "sender_id": "string",
        "content": "string",
        "timestamp": "datetime"
      }
    ]
  }
  ```

### 2. **Send a Message**

- **Method**: `POST`
- **Endpoint**: `/api/message/send`
- **Description**: Send a message to another user.
- **Request Body**:

  ```json
  {
    "recipient_id": "string",
    "content": "string"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Message sent successfully"
  }
  ```

---

## Social Features

### 1. **Follow User**

- **Method**: `POST`
- **Endpoint**: `/api/user/{user_id}/follow`
- **Description**: Follow a specific user.
- **Response**:

  ```json
  {
    "message": "Followed successfully"
  }
  ```

### 2. **Unfollow User**

- **Method**: `DELETE`
- **Endpoint**: `/api/user/{user_id}/unfollow`
- **Description**: Unfollow a specific user.
- **Response**:

  ```json
  {
    "message": "Unfollowed successfully"
  }
  ```

### 3. **Like Post**

- **Method**: `POST`
- **Endpoint**: `/api/forum/post/{post_id}/like`
- **Description**: Like a specific post.
- **Response**:

  ```json
  {
    "message": "Post liked successfully"
  }
  ```

### 4. **Unlike Post**

- **Method**: `DELETE`
- **Endpoint**: `/api/forum/post/{post_id}/like`
- **Description**: Remove like from a specific post.
- **Response**:

  ```json
  {
    "message": "Post unliked successfully"
  }
  ```

---

## Draft Management

### 1. **Get Drafts**

- **Method**: `GET`
- **Endpoint**: `/api/draft`
- **Description**: Retrieve the list of drafts for the current user.
- **Response**:

  ```json
  {
    "drafts": [
      {
        "id": "int",
        "title": "string",
        "content": "string",
        "media": ["url"],
        "timestamp": "datetime"
      }
    ]
  }
  ```

### 2. **Create Draft**

- **Method**: `POST`
- **Endpoint**: `/api/draft`
- **Description**: Create a new draft.
- **Request Body**:

  ```json
  {
    "title": "string",
    "content": "string",
    "media": ["url"]
  }
  ```

- **Response**:

  ```json
  {
    "message": "Draft created successfully",
    "draft_id": "int"
  }
  ```

### 3. **Edit Draft**

- **Method**: `PUT`
- **Endpoint**: `/api/draft`
- **Description**: Edit an existing draft.
- **Request Body**:

  ```json
  {
    "draft_id": "int",
    "title": "string",
    "content": "string",
    "media": ["url"]
  }
  ```

- **Response**:

  ```json
  {
    "message": "Draft updated successfully"
  }
  ```

### 4. **Delete Draft**

- **Method**: `DELETE`
- **Endpoint**: `/api/draft`
- **Description**: Delete a specific draft.
- **Request Body**:

  ```json
  {
    "draft_id": "int"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Draft deleted successfully"
  }
  ```

### 5. **Upload Draft Media**

- **Method**: `POST`
- **Endpoint**: `/api/draft/media`
- **Description**: Upload media files for a draft.
- **Request Body**:
  - `Content-Type`: `multipart/form-data`
  - Field Name: `media`
- **Response**:

  ```json
  {
    "message": "Media uploaded successfully"
  }
  ```

### 6. **Publish Draft**

- **Method**: `POST`
- **Endpoint**: `/api/draft/publish`
- **Description**: Publish a draft to create a post.
- **Request Body**:

  ```json
  {
    "draft_id": "int"
  }
  ```

- **Response**:

  ```json
  {
    "message": "Draft published successfully",
    "post_id": "int"
  }
  ```

---
