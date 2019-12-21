<?php
require 'info.php';

$username = urldecode($_GET['username']);

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT UserID, avatar FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$id = $row['UserID'];
	$avatar = $row['avatar'];
	
	echo "AVATAR::".$avatar."USER::ID::$id";
}else{
    echo "An error has occurred.";
}
?>