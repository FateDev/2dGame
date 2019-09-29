<?php
require 'info.php';

$id = urldecode($_GET['id']);
$accessToken = urldecode($_GET['token']);

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT sessionToken FROM usersVerified WHERE UserID=:id");
$statement->bindParam(':id', $id, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$row = $statement->fetch(PDO::FETCH_ASSOC);
	$realToken = $row['sessionToken'];
	
	if($realToken == $accessToken){
        $statement = $pdo->prepare("UPDATE usersVerified SET sessionToken='', loggedIn=0 WHERE UserID=:id");
        $statement->bindParam(':id', $id, PDO::PARAM_STR);
        $statement->execute();

        echo "success";
    }else{
        echo "failure";
    }
}else{
    echo "failure";
    exit();
}

?>