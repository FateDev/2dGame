<?php
require 'info.php';

$username = urldecode($_GET['username']);
$accessToken = urldecode($_GET['token']);

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT pword, UserID FROM usersVerified WHERE username=:username AND sessionToken=:token");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->bindParam(':token', $accessToken, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$id = $row['UserID'];
	
	echo "USER::ID::$id";
}else{
    echo "An error has occurred.";
}
?>