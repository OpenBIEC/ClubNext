function isUserLoggedIn()
{
    return fetch('/api/user/profile', {
               method : 'GET',
               credentials : 'include',
           })
        .then(response => {
            if (response.status === 401)
            {
                return false;
            }
            if (!response.ok)
            {
                throw new Error('Error checking user login status');
            }
            return true;
        })
        .catch(error => {
            console.error('Error checking login status:', error);
            return false;
        });
}

function fetchUserProfile()
{
    return fetch('/api/user/profile', {
               method : 'GET',
               credentials : 'include',
           })
        .then(response => {
            if (!response.ok)
            {
                throw new Error('Failed to fetch user profile');
            }
            return response.json();
        })
        .then(data => {
            return {
                username : data.username,
                avatar : data.avatar,
                bio : data.bio,
                followers : data.followers,
                following : data.followings,
                joinedDate : new Date(data.joined_at).toLocaleDateString(),
            };
        })
        .catch(error => {
            console.error('Error fetching user profile:', error);
            return null;
        });
}