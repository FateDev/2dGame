<?php
require 'info.php';

function generateRandomString($length = 20) {
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $charactersLength = strlen($characters);
    $randomString = '';
    for ($i = 0; $i < $length; $i++) {
        $randomString .= $characters[rand(0, $charactersLength - 1)];
    }
    return $randomString;
}

$username = urldecode($_GET['username']);
$password = urldecode($_GET['password']);

if(strlen($password) > 20){
    echo "false1";
	exit();
}

if(strlen($username) > 20){
    echo "false2";
	exit();
}

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT pword, UserID, loggedIn FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$realPassword = $row['pword'];
   $id = $row['UserID'];
   $loggedIn = $row['loggedIn'];

    if($loggedIn == 1){
        echo "false3";
        exit();
    }

	if(password_verify($password, $realPassword)){
        $token = generateRandomString();

	    $statement = $pdo->prepare("UPDATE usersVerified SET sessionToken=:token, loggedIn=1 WHERE UserID=:id");
	    $statement->bindParam(':id', $id, PDO::PARAM_STR);
        $statement->bindParam(':token', $token, PDO::PARAM_STR);
	    $statement->execute();

		echo $token;
	}else{
	    echo "false4";
    	exit();
	}
}else{
    echo "false5";
    exit();
}
?>
