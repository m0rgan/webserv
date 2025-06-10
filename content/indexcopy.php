#!/usr/bin/php-cgi
<?php


session_start();

// User storage (plain text, just for testing)
$userFile = "users.json";
if (!file_exists($userFile)) {
    file_put_contents($userFile, json_encode([]));
}
$users = json_decode(file_get_contents($userFile), true);

// Handle registration & login
if ($_SERVER["REQUEST_METHOD"] === "POST") {
	header("Content-Type: application/json");
    $input = file_get_contents("php://input");
    $data = json_decode($input, true);
    
    $username = trim($data['username'] ?? "");
    $password = trim($data['password'] ?? "");

    if (!empty($username) && !empty($password)) {
        if (isset($users[$username])) {
            if ($users[$username] === $password) {
                $_SESSION['LOGGED_IN'] = true;
                $_SESSION['USERNAME'] = $username;
				echo json_encode(["success" => "Logged in"]);
            } else {
                echo json_encode(["error" => "Invalid password"]);
            }
        } else {
            $users[$username] = $password;
            file_put_contents($userFile, json_encode($users));
            $_SESSION['LOGGED_IN'] = true;
            $_SESSION['USERNAME'] = $username;
			echo json_encode(["success" => "Registered"]);
        }
    } else {
		echo json_encode(["error" => "Username and password required"]);
	}
	exit;
}

// Handle logout
if ($_SERVER["REQUEST_METHOD"] === "DELETE") {
    header("Content-Type: application/json");
	session_destroy();
	setcookie("PHPSESSID", "", time() - 3600, "/");
    echo json_encode(["success" => "Logged out"]);
    exit;
}
header("Content-Type: text/html");
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP CGI Login</title>
    <style>
        body { background-color: #FFD700; text-align: center; color: black; font-family: Arial, sans-serif; }
        img { width: 25%; display: block; margin: 20px auto; }
        input, button { margin: 10px; padding: 10px; font-size: 16px; }
    </style>
</head>
<body>
    <img src="https://www.php.net/images/logos/php-logo.svg" alt="PHP Logo">
    <h1>PHP CGI Login</h1>
    <form id="loginForm">
        <input type="text" id="username" placeholder="Username">
        <input type="password" id="password" placeholder="Password">
        <button type="submit">Register/Login</button>
    </form>
    <button onclick="logout()">Logout</button>
    <p id="status"></p>

    <script>
        document.getElementById("loginForm").addEventListener("submit", async function(event) {
            event.preventDefault();
            const username = document.getElementById("username").value;
            const password = document.getElementById("password").value;

            const response = await fetch("/indexcopy.php", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ username, password })
            });

            const result = await response.json();
            document.getElementById("status").textContent = result.error || "Logged in!";
            
        });

        async function logout() {
            const response = await fetch("/indexcopy.php", {
                method: "DELETE",
                credentials: "include"
            });
            const result = await response.json();
            document.getElementById("status").textContent = result.error || "Logged out!";
        }

    </script>
</body>
</html>
