<?php
require 'info.php';

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);
$output = "";

if($_GET['save'] == "true") {
    $id = $_GET['id'];
    $msg = urldecode($_GET['msg']);
    $roomGuild = $_GET['roomGuild'];
    $time = $_GET['time'];

    $statement = "INSERT INTO `messages` (`msg`, `timeSeconds`, `fromID`, `roomGuild`) VALUES (?, ?, ?, ?);";
    $pdo->prepare($statement)->execute([$msg, $time, $id, $roomGuild]);
    
    $statement = $pdo->prepare("SELECT id FROM `messages` WHERE `fromID`=:id ORDER BY id DESC LIMIT 1");
    $statement->bindParam(':id', $id, PDO::PARAM_INT);
    $statement->execute();
    
	$row = $statement->fetch(PDO::FETCH_ASSOC);
    $msgID = $row['id'];

    echo $msgID;

	exit();
} else {
    $roomGuild = $_GET['roomGuild'];

    $statement = $pdo->prepare("SELECT * FROM `messages` WHERE roomGuild=:roomGuild ORDER BY id DESC LIMIT 50");
    $statement->bindParam(':roomGuild', $roomGuild, PDO::PARAM_STR);
    $statement->execute();

    $list = array();
    while($row = $statement->fetch(PDO::FETCH_ASSOC)){
        $msg = $row['msg'];
        $time = $row['timeSeconds'];
        $from = $row['fromID'];
        $msgID = $row['id'];

        $sql = $pdo->prepare("SELECT username FROM `usersVerified` WHERE `UserID`=:id");
        $sql->bindParam(':id', $from, PDO::PARAM_INT);
        $sql->execute();
        
        $row = $sql->fetch(PDO::FETCH_ASSOC);
        $from = $row['username'];

        if($imgURL == ""){
            $imgURL = "EMPTY";
        }

        $msgObj = new StdClass();
        $msgObj->imgURL = $imgURL;
        $msgObj->msgID = $msgID;
        $msgObj->time = $time;
        $msgObj->from = $from;
        $msgObj->msg = $msg;

        array_push($list, $msgObj);
    }
    echo json_encode($list);

    exit();
}
?>