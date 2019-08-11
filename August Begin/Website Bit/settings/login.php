<?php
require '../info.php';
session_start();

if(isset($_POST['username']) && isset($_POST['password'])){
    $username = $_POST['username'];
    $password = $_POST['password'];
}else{
    header('Location: index.php?error=true');
	exit();
}

if(strlen($password) > 20){
    header('Location: index.php?error=passwordError');
	exit();
}

if(strlen($username) > 20){
    header('Location: index.php?error=invalidUsername');
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
		$_SESSION['userGame']['id'] = $id;
		$_SESSION['userGame']['username'] = $username;
		$_SESSION['userGame']['password'] = $password;
	}else{
		header('Location: index.php?error=true');
	}
}else{
	header('Location: index.php?error=true');
}

header('Location: index.php');
?>