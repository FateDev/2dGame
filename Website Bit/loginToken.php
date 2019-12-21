<?php
require 'info.php';

$username = urldecode($_GET['username']);
$password = urldecode($_GET['password']);

if(strlen($password) > 20){
    echo "false";
	exit();
}

if(strlen($username) > 20){
    echo "false";
	exit();
}

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT pword, UserID FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$realPassword = $row['pword'];
	$id = $row['UserID'];

	if(password_verify($password, $realPassword)){
		echo "true";
	}else{
	    echo "false";
    	exit();
	}
}else{
    echo "false";
    exit();
}
?>
