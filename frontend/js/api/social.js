async function likePost(postId)
{
    const endpoint = `/api/forum/post/${postId}/like`;
    try
    {
        const response = await fetch(endpoint, {
            method : 'POST',
            credentials : 'include',
        });
        if (!response.ok)
        {
            throw new Error('Failed to like the post');
        }
        return await response.json(); // 返回后端响应数据
    }
    catch (error)
    {
        console.error(`Error liking post ${postId}:`, error);
        throw error;
    }
}

async function unlikePost(postId)
{
    const endpoint = `/api/forum/post/${postId}/like`;
    try
    {
        const response = await fetch(endpoint, {
            method : 'DELETE',
            credentials : 'include',
        });
        if (!response.ok)
        {
            throw new Error('Failed to unlike the post');
        }
        return await response.json(); // 返回后端响应数据
    }
    catch (error)
    {
        console.error(`Error unliking post ${postId}:`, error);
        throw error;
    }
}
