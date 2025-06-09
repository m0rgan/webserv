#!/usr/bin/python3

import os
import sys
import json
import http.cookies

# Paths for session and user storage
SESSION_DIR = "sessions"
USER_DB = "users.json"

# Ensure directories exist
if not os.path.exists(SESSION_DIR):
    os.makedirs(SESSION_DIR)

# Load user database (plain text for testing)
if not os.path.exists(USER_DB):
    with open(USER_DB, "w") as f:
        json.dump({}, f)

with open(USER_DB, "r") as f:
    try:
        users = json.load(f)
    except json.JSONDecodeError:
        users = {}

# Read cookies
cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
session_id = cookie["SESSIONID"].value if "SESSIONID" in cookie else None

# Create a new session if none exists
if not session_id:
    session_id = str(len(os.listdir(SESSION_DIR)) + 1)  # Simple unique ID
    print(f"Set-Cookie: SESSIONID={session_id}; Path=/; HttpOnly")

# Define session file
session_file = os.path.join(SESSION_DIR, f"{session_id}.json")

# Load session data
session_data = {"logged_in": False, "username": ""}
if os.path.exists(session_file):
    with open(session_file, "r") as f:
        session_data = json.load(f)

# Read request method
request_method = os.environ.get("REQUEST_METHOD", "GET")

# Handle registration & login
if request_method == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    input_data = sys.stdin.read(content_length)
    
    try:
        data = json.loads(input_data)
    except json.JSONDecodeError:
        print("Content-Type: application/json\r\n")
        print(json.dumps({"error": "Invalid JSON"}))
        sys.exit(0)

    username = data.get("username", "").strip()
    password = data.get("password", "").strip()

    if username and password:
        if username in users:
            if users[username] == password:
                session_data["logged_in"] = True
                session_data["username"] = username
                with open(session_file, "w") as f:
                    json.dump(session_data, f)
            else:
                print("Content-Type: application/json\r\n")
                print(json.dumps({"error": "Invalid password"}))
                sys.exit(0)
        else:
            users[username] = password
            with open(USER_DB, "w") as f:
                json.dump(users, f)
            session_data["logged_in"] = True
            session_data["username"] = username
            with open(session_file, "w") as f:
                json.dump(session_data, f)

# Handle logout
elif request_method == "DELETE":
    if os.path.exists(session_file):
        try:
            os.remove(session_file)
        except OSError:
            pass  # Handle potential errors

    # Force cookie removal with Max-Age=0
    print("Set-Cookie: SESSIONID=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0; HttpOnly")
    print("Content-Type: application/json\r\n")
    print(json.dumps({"message": "Logged out"}))
    sys.exit(0)


# Output headers
print("Content-Type: text/html\r\n")
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Python CGI Login</title>
    <style>
        body {{ background-color: #000080; text-align: center; color: white; font-family: Arial, sans-serif; }}
        img {{ width: 25%; display: block; margin: 20px auto; }}
        input, button {{ margin: 10px; padding: 10px; font-size: 16px; }}
    </style>
</head>
<body>
    <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg" alt="Python Logo">
    <h1>Python CGI Login</h1>
    <form id="loginForm">
        <input type="text" id="username" placeholder="Username">
        <input type="password" id="password" placeholder="Password">
        <button type="submit">Register/Login</button>
    </form>
    <button onclick="logout()">Logout</button>
    <p id="status"></p>

    <script>
        document.getElementById("loginForm").addEventListener("submit", async function(event) {{
            event.preventDefault();
            const username = document.getElementById("username").value;
            const password = document.getElementById("password").value;

            const response = await fetch("/indexcopy.py", {{
                method: "POST",
                headers: {{ "Content-Type": "application/json" }},
                body: JSON.stringify({{ username, password }})
            }});

            const result = await response.json();
            document.getElementById("status").textContent = result.error || "Logged in!";
            if (!result.error) location.reload();
        }});
        async function logout() {{
            const response = await fetch("/indexcopy.py", {{ method: "DELETE" }});
            const result = await response.json();
            console.log(result.message); // Debugging: Check if logout response is received
            document.cookie = "SESSIONID=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/"; // Force cookie deletion
            location.reload();
        }}

    </script>
</body>
</html>
""")

# --- Close file descriptors before exiting ---
def close_fds():
    """ Close all file descriptors (≥3) after script execution """
    for fd in range(3, 1024):
        try:
            os.close(fd)
        except OSError:
            pass  # Ignore errors for already closed FDs

# Register function to run at script shutdown
import atexit
atexit.register(close_fds)
