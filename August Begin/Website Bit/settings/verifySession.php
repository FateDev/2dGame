<?php
require '../info.php';
session_start();

if(isset($_SESSION['userGame']['username']) && isset($_SESSION['userGame']['password']) && isset($_SESSION['userGame']['id'])){
    $id = $_SESSION['userGame']['id'];
    $password = $_SESSION['userGame']['password'];
}else{
    $sessionActive = false;
}

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT pword FROM usersVerified WHERE UserID=:id");
$statement->bindParam(':id', $id, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$realPassword = $row['pword'];
	
	if(password_verify($password, $realPassword)){
		$sessionActive = true;
	}else{
		$sessionActive = false;
	}
}else{
	$sessionActive = false;
}

?>