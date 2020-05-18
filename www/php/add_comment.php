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
$new_id = getenv('new_id');
$comment = getenv('comment');
$conn = ConnectDB("tinyserver");
$sql = "insert into comment(comment, new_id) values('$comment', $new_id)";
if ($conn->query($sql) == TRUE)
{
	echo "<script>window.location.href='../new.php?id=$new_id';</script>";
}
?>