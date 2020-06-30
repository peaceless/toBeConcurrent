<?php
function ConnectDB($dbname)
{
	$servername = "127.0.0.1";
	$username = "test";
	$password = "123";
	$conn = new mysqli($servername, $username, $password, $dbname);
	if ($conn->connect_error) {
		die("fail to connect: " . $conn->connect_error);
	}
	return $conn;
}
session_start();

$username = getenv('username');
$password = getenv('password');

$conn = ConnectDB('tinyserver');
$sql = 'select * from user where username=\''.$username.'\'';
$result = $conn->query($sql);
$signup = TRUE;
if ($row = $result->fetch_assoc())
{
	$signup=FALSE;
}

if ($signup)
{
	$_SESSION['username'] = $username;
	$sql = "insert into user(username, password) values('$username', '$password')";
	$conn->query($sql);
	$msid = session_id();
	echo '<script>window.location.href=\'../user_page.php?msid=',$msid,'\';</script>';
}
else
{
	echo "<script>alert('username exist');window.location.href='../signup.html';</script>";
}
?>