function isUserLoggedIn()
{
    return fetch('/api/user/profile', {
               method : 'GET',
               credentials : 'include' // Ensures cookies are sent with the request
           })
        .then(response => {
            if (response.status === 401)
            {
                return false; // User not logged in
            }
            if (!response.ok)
            {
                throw new Error('Error checking user login status');
            }
            return true; // User logged in
        })
        .catch(error => {
            console.error('Error checking login status:', error);
            return false; // Assume not logged in on error
        });
}

function fetchUserProfile()
{
    return fetch('/api/user/profile', {
               method : 'GET',
               credentials : 'include' // Ensures cookies are sent with the request
           })
        .then(response => {
            if (!response.ok)
            {
                throw new Error('Failed to fetch user profile');
            }
            return response.json(); // Parse the JSON response
        })
        .then(data => {
            // Return a structured user profile object
            return {
                username : data.username,
                avatar : data.avatar,
                bio : data.bio,
                followers : data.followers,
                following : data.followings,
                joinedDate : new Date(data.joined_at).toLocaleDateString()
            };
        })
        .catch(error => {
            console.error('Error fetching user profile:', error);
            return null; // Return null on error
        });
}

async function sendVerificationCode(username, email)
{
    try
    {
        const response = await fetch(
            `/api/user/verify?username=${encodeURIComponent(username)}&email=${encodeURIComponent(email)}`, {
                method : 'GET',
                credentials : 'include',
            });
        const data = await response.json();

        if (response.ok)
        {
            alert(data.message || 'Verification code sent successfully!');
        }
        else
        {
            alert(data.error || 'Failed to send verification code.');
        }
    }
    catch (error)
    {
        console.error('Error sending verification code:', error);
    }
}