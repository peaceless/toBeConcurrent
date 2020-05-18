<?php
session_start();
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

//get session

$msid = session_id();	
$username = getenv('username');
$password = getenv('password');

$conn = ConnectDB('tinyserver');
$sql = 'select * from user where username=\''.$username.'\' and password=\''.$password.'\'';

$result = $conn->query($sql);
$login = FALSE;
if ($row = $result->fetch_assoc())
{
	$login=TRUE;
}

if ($login)
{
	$_SESSION['username'] = $username;
	echo '<script>window.location.href=\'../user_page.php?msid=',$msid,'\';</script>';
}
else
{
	echo '<script>window.location.href=\'../login.html\'</script>';
}
?>
