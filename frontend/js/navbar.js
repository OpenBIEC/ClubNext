async function updateNavBar()
{
    try
    {
        const isLoggedIn = await isUserLoggedIn();

        const navItems = document.getElementById('navItems');
        navItems.innerHTML = ''; // Clear existing nav items

        if (isLoggedIn)
        {
            // If the user is logged in, show Profile, Messages, and Posts
            navItems.innerHTML = `
                <li class="nav-item">
                    <a class="nav-link" href="/profile.html">
                        <i class="bi bi-person-circle"></i> Profile
                    </a>
                </li>
                <li class="nav-item">
                    <a class="nav-link" href="/private_message.html">
                        <i class="bi bi-chat-dots"></i> Messages
                    </a>
                </li>
                <li class="nav-item">
                    <a class="nav-link" href="/post.html">
                        <i class="bi bi-file-earmark-text"></i> Posts
                    </a>
                </li>
            `;
        }
        else
        {
            // If the user is not logged in, show Sign In and Sign Up
            navItems.innerHTML = `
                <li class="nav-item">
                    <a class="nav-link" href="/login.html">
                        <i class="bi bi-box-arrow-in-right"></i> Sign In
                    </a>
                </li>
                <li class="nav-item">
                    <a class="nav-link" href="/register.html">
                        <i class="bi bi-pencil-square"></i> Sign Up
                    </a>
                </li>
            `;
        }
    }
    catch (error)
    {
        console.error('Error updating navbar:', error);
    }
}