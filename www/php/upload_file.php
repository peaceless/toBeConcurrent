<!DOCTYPE html>
<html>
<head>
	<title></title>
</head>
<body>
<?php 
	$filename = "tt.ico";
	$file_data = getenv('file');
	//$file = fopen($filename, "w");
	//file_put_contents($file, $file_data);
	echo "<p>$file_data</p>";
 ?>
</body>
</html>
