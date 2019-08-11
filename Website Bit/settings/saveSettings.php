<?php
require 'verifySession.php';

if($sessionActive == false){
	header('Location: index.php?error=notLoggedIn');
	exit();
}

if(!isset($_POST['username']) || !isset($_POST['password'])){
	header('Location: settings.php?error=incorrectDetails');
	exit();
}else{	
	$username = $_POST['username'];
	$password = $_POST['password'];
}

if(isset($_POST['newPass']) && isset($_POST['newPassRepeat']) && $_POST['newPass'] != "" && $_POST['newPassRepeat'] != ""){
	$newPass = $_POST['newPass'];
	$newPassRepeat = $_POST['newPassRepeat'];
	$updatePass = true;
	
	if($newPass != $newPassRepeat){
		header('Location: settings.php?error=passNotMatch');
		exit();
	}
}else{
	$updatePass = false;
}

if(isset($_POST['privateMessaging'])){
	$privateMessaging = true;
}else{
	$privateMessaging = false;
}

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT pword FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$realPassword = $row['pword'];
	
	if(!password_verify($password, $realPassword)){
		header('Location: settings.php?error=incorrectDetails');
		exit();
	}
}else{
	header('Location: settings.php?error=incorrectDetails');
	exit();
}

if($updatePass){
	if(strlen($newPass) > 20 || strlen($newPass) < 5){
		header('Location: settings.php?error=passwordError');
		exit();
	}
	
	$passwordHash = password_hash($newPass, PASSWORD_DEFAULT);
	
	$statement = $pdo->prepare("UPDATE usersVerified  SET pword=:password WHERE UserID=:id");
	$statement->bindParam(':password', $passwordHash, PDO::PARAM_STR);
	$statement->bindParam(':id', $id, PDO::PARAM_STR);
	$statement->execute();
	
	$_SESSION['userGame']['password'] = $newPass;
}

$statement = $pdo->prepare("SELECT settings FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

$settings = $statement->fetch(PDO::FETCH_ASSOC);
$settings = json_decode($settings['settings'], true);

$settings['privateMessaging'] = $privateMessaging;

echo $settings['custom_binding1']['action'];

$settings = json_encode($settings);

echo $settings;

$statement = $pdo->prepare("UPDATE usersVerified  SET settings=:settings WHERE UserID=:id");
$statement->bindParam(':settings', $settings, PDO::PARAM_STR);
$statement->bindParam(':id', $id, PDO::PARAM_STR);
$statement->execute();

header('Location: settings.php?success=saved');
?>