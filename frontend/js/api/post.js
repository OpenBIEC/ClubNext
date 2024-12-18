async function fetchRecommendedPostIds(limit = 5)
{
    try
    {
        const response = await fetch(`/api/forum/recommend?limit=${limit}`, {
            method : 'GET',
            credentials : 'include',
        });
        if (!response.ok)
        {
            throw new Error('Failed to fetch recommended post IDs');
        }
        const data = await response.json();
        return data.recommended_posts.map(post => post.id);
    }
    catch (error)
    {
        console.error('Error fetching recommended post IDs:', error);
        return [];
    }
}

async function fetchPostDetails(postId)
{
    try
    {
        const response = await fetch(`/api/forum/post/${postId}`, {
            method : 'GET',
            credentials : 'include',
        });
        if (!response.ok)
        {
            throw new Error(`Failed to fetch post details for post ID ${postId}`);
        }
        return await response.json();
    }
    catch (error)
    {
        console.error(`Error fetching details for post ID ${postId}:`, error);
        return null;
    }
}

async function generatePostHTML({id, author, content, media, like_count, liked_by_user, comment_count})
{
    let parsedContent = '';

    try
    {
        const response = await fetch(content);
        if (response.ok)
        {
            const markdown = await response.text();
            parsedContent = marked.parse(markdown);
        }
        else
        {
            parsedContent = '<p class="text-danger">Failed to load content.</p>';
        }
    }
    catch (error)
    {
        console.error('Error fetching content:', error);
        parsedContent = '<p class="text-danger">Error loading content.</p>';
    }

    return `
        <div class="mb-3">
            <div class="card">
                <div class="card-body">
                    <div class="d-flex align-items-center mb-2">
                        <img src="${media?.[0] || 'https://placehold.co/50'}" alt="${
        author}'s Avatar" class="rounded-circle me-2" style="width: 40px; height: 40px;">
                        <h6 class="card-subtitle mb-0 text-muted">@${author}</h6>
                    </div>
                    <div class="card-text markdown-content">${parsedContent}</div>
                    <div class="d-flex mt-3">
                        <button class="btn me-2 like-button ${liked_by_user ? "text-danger" : "text-muted"}" data-id="${
        id}">
                            <i class="bi ${liked_by_user ? "bi-heart-fill" : "bi-heart"}"></i> ${like_count}
                        </button>
                        <button class="btn comment-button text-muted" data-id="${id}">
                            <i class="bi bi-chat"></i> ${comment_count}
                        </button>
                    </div>
                </div>
            </div>
        </div>
    `;
}

async function loadMorePosts(postList, loadingIndicator, limit = 5)
{
    try
    {
        loadingIndicator.style.display = "block";
        const recommendedPostIds = await fetchRecommendedPostIds(limit);

        if (recommendedPostIds.length === 0)
        {
            loadingIndicator.style.display = "none";
            return false;
        }

        for (const postId of recommendedPostIds)
        {
            const postDetails = await fetchPostDetails(postId);
            if (postDetails)
            {
                postList.insertAdjacentHTML("beforeend", await generatePostHTML(postDetails));
            }
        }

        return true;
    }
    catch (error)
    {
        console.error('Error loading posts:', error);
        return false;
    }
    finally
    {
        loadingIndicator.style.display = "none";
    }
}

async function fetchTags()
{
    const endpoint = '/api/tags';
    try
    {
        const response = await fetch(endpoint, {
            method : 'GET',
            credentials : 'include',
        });
        if (!response.ok)
        {
            throw new Error('Failed to fetch tags');
        }
        return await response.json();
    }
    catch (error)
    {
        console.error('Error fetching tags:', error);
        throw error;
    }
}

async function createPost(content)
{
    const endpoint = '/api/forum/post';
    try
    {
        const response = await fetch(endpoint, {
            method : 'POST',
            headers : {
                'Content-Type' : 'application/json',
            },
            credentials : 'include',
            body : JSON.stringify({content}),
        });
        if (!response.ok)
        {
            throw new Error('Failed to create post');
        }
        return await response.json();
    }
    catch (error)
    {
        console.error('Error creating post:', error);
        throw error;
    }
}

async function uploadMedia(postId, file, type)
{
    const endpoint = `/api/forum/post/${postId}/media`;
    const formData = new FormData();
    formData.append(type, file);

    try
    {
        const response = await fetch(endpoint, {
            method : 'POST',
            credentials : 'include',
            body : formData,
        });
        if (!response.ok)
        {
            throw new Error('Failed to upload media');
        }
        return await response.json();
    }
    catch (error)
    {
        console.error(`Error uploading ${type} for post ${postId}:`, error);
        throw error;
    }
}

async function updatePost(postId, content)
{
    const endpoint = `/api/forum/post/${postId}`;
    try
    {
        const response = await fetch(endpoint, {
            method : 'POST',
            headers : {
                'Content-Type' : 'application/json',
            },
            credentials : 'include',
            body : JSON.stringify({content}),
        });
        if (!response.ok)
        {
            throw new Error('Failed to update post');
        }
        return await response.json();
    }
    catch (error)
    {
        console.error(`Error updating post ${postId}:`, error);
        throw error;
    }
}
