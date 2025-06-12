#!/usr/bin/python3

import os
import sys
import json
import http.cookies

USER_DB = "users.json"

# Load or create user database
if not os.path.exists(USER_DB):
    with open(USER_DB, "w") as f:
        json.dump({}, f)

with open(USER_DB, "r") as f:
    try:
        users = json.load(f)
    except json.JSONDecodeError:
        users = {}

headers = []
username = None
valid_session = False

# Parse cookies
cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
session_cookie = cookie.get("SESSIONID")
if not session_cookie:
    session_cookie = os.environ.get("SESSIONID")

if session_cookie:
    username = session_cookie.value
    if username in users:
        valid_session = True
else:
    headers.append("<!-- DEBUG: No SESSIONID cookie found -->")

request_method = os.environ.get("REQUEST_METHOD", "GET")

# --- Handle POST (register/login) ---
if request_method == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    input_data = sys.stdin.read(content_length)

    try:
        data = json.loads(input_data)
    except json.JSONDecodeError:
        headers.append("Content-Type: application/json")
        print("\r\n".join(headers) + "\r\n")
        print(json.dumps({"error": "Invalid JSON"}))
        sys.exit(0)

    input_username = data.get("username", "").strip()
    password = data.get("password", "").strip()

    if input_username and password:
        if input_username in users:
            if users[input_username] == password:
                headers.append(f"Set-Cookie: SESSIONID={input_username}; Path=/; HttpOnly")
                headers.append("Content-Type: application/json")
                print("\r\n".join(headers) + "\r\n")
                print(json.dumps({"message": "Logged in!"}))
                sys.exit(0)
            else:
                headers.append("Content-Type: application/json")
                print("\r\n".join(headers) + "\r\n")
                print(json.dumps({"error": "Invalid password"}))
                sys.exit(0)
        else:
            users[input_username] = password
            with open(USER_DB, "w") as f:
                json.dump(users, f)
            headers.append(f"Set-Cookie: SESSIONID={input_username}; Path=/; HttpOnly")
            headers.append("Content-Type: application/json")
            print("\r\n".join(headers) + "\r\n")
            print(json.dumps({"message": "Registered!"}))
            sys.exit(0)

# --- Handle DELETE (logout) ---
elif request_method == "DELETE":
    headers.append("Set-Cookie: SESSIONID=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0; HttpOnly")
    headers.append("Content-Type: application/json")
    print("\r\n".join(headers) + "\r\n")
    print(json.dumps({"message": "Logged out"}))
    sys.exit(0)

# --- HTML Page (GET) ---
status_message = ""
auth_form = ""
logout_button = ""
welcome_message = ""
secret_content = ""

if valid_session:
    welcome_message = f"<h2>Welcome, {username}!</h2>"
    status_message = "You are currently logged in."
    secret_content = "<p>This is secret content visible only when logged in!</p>"
    logout_button = '<button onclick="logout()">Logout</button>'
else:
    status_message = "You are not logged in."
    auth_form = """
    <form id="authForm">
        <input type="text" id="username" placeholder="Username" required>
        <input type="password" id="password" placeholder="Password" required>
        <button type="submit">Register/Login</button>
    </form>
    """

headers.append("Content-Type: text/html")
print("\r\n".join(headers) + "\r\n")

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Python CGI Login</title>
    <style>
        body {{ background-color: #000080; text-align: center; color: white; font-family: Arial, sans-serif; }}
        img {{ width: 25%; display: block; margin: 20px auto; }}
        input, button {{ margin: 10px; padding: 10px; font-size: 16px; }}
        .logged-in-content {{ margin-top: 30px; border: 1px solid lime; padding: 20px; }}
        .hidden {{ display: none; }}
    </style>
</head>
<body>
    <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg" alt="Python Logo">
    <h1>Python CGI Login</h1>

    {welcome_message}
    {auth_form}
    {logout_button}

    <p id="status">{status_message}</p>

    <div id="secretContent" class="{'' if valid_session else 'hidden'} logged-in-content">
        {secret_content}
    </div>

    <script>
        document.getElementById("authForm")?.addEventListener("submit", async function(event) {{
            event.preventDefault();
            const username = document.getElementById("username").value;
            const password = document.getElementById("password").value;

            const response = await fetch("/indexcopy.py", {{
                method: "POST",
                headers: {{ "Content-Type": "application/json" }},
                credentials: 'same-origin',
                body: JSON.stringify({{ username, password }})
            }});

            const result = await response.json();
            console.log("Login/Register Response:", result);
            document.getElementById("status").textContent = result.message || result.error || "Unknown response";
            if (response.ok && (result.message === "Logged in!" || result.message === "Registered!")) {{
                window.location.reload();
            }}
        }});

        async function logout() {{
            const response = await fetch("/indexcopy.py", {{
                method: "DELETE",
                credentials: 'same-origin'
            }});
            const result = await response.json();
            console.log("Logout Response:", result);
            document.getElementById("status").textContent = result.message || result.error || "Unknown response";
            document.cookie = "SESSIONID=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/";
            if (response.ok && result.message === "Logged out") {{
                window.location.reload();
            }}
        }}
    </script>
</body>
</html>
""")
