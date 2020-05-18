<?php
	$msid = getenv('msid');
	session_id($msid);
	session_start();
	$username = $_SESSION['username'];
	if($_SESSION['username']=="")
	{
		echo "<script>alert('please login');window.location.href='login.html';</script>";
		exit();
	}
?>
<html>
<body>
	<?php
		echo $username;
		echo '<br>welcome';
	 ?>
</body>
</html>