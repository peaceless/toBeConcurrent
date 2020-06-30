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
?>
<?php
$news_id = getenv('id');

if ($news_id == NULL)
{
	echo "<h2>震惊新闻</h2>";
	$json_str = file_get_contents("./news/list.json");
	$data = json_decode($json_str, true);
	echo "<ul>";
	foreach ($data['newlist'] as $value){
		$newid = $value['id'];
		$desc = $value['desc'];
		$link = "<li><a href='new.php?id=$newid'>$desc</a></li>";
		echo $link;
	}
	echo "</ul>";
}else{
	$sql = "select * from news where id=$news_id";
	$conn = ConnectDB("tinyserver");
	$result = $conn->query($sql);
	$filename = "";
	if ($row = $result->fetch_assoc())
	{
		$filename = "./news/".$row['new'];
		$file = fopen($filename, "r");
		$data = fread($file, filesize($filename));

		echo "<center>";
		echo $data;
		fclose($file);

		echo '<form action="php/add_comment.php?new_id=',$news_id,'" method="post">
				<h3>评论</h3>
				<textarea rows="3" cols="40" name="comment"></textarea>
				<br>
        		<input type="submit" value="comment" />
        		</form>';
        $sql = "select * from comment where new_id=$news_id";
        $result = $conn->query($sql);
        echo "<div style= 'font-size: 30px;'>";
        while ($row = $result->fetch_assoc())
        {
        	$comment = $row['comment'];
        	echo "<p>",$comment,"</p>";
        }
        echo "</div>";
		echo "</center>";
	}
}
?>	
