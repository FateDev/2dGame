<?php
require 'info.php';

$userID = urldecode($_GET['id']);

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);
$output = "";

if(!is_numeric($_GET['id'])){
    $statement = $pdo->prepare("SELECT * FROM `roomGuild`");
    $statement->execute();

    $list = array();
    while($row = $statement->fetch(PDO::FETCH_ASSOC)){
        $name = $row['name'];
        $id = $row['id'];

        $returnObj = new StdClass();
        $returnObj->name = $name;
        $returnObj->id = $id;

        array_push($list, $returnObj);
    }

    echo json_encode($list);
}else{
    $statement = $pdo->prepare("SELECT `settings` FROM `usersVerified` WHERE `UserID`=:id");
    $statement->bindParam(':id', $userID, PDO::PARAM_INT);
    $statement->execute();

    $row = $statement->fetch(PDO::FETCH_ASSOC);

    $settings = json_decode($row['settings']);
    $guilds = explode("#", $settings->guilds);

    $list = array();
    foreach ($guilds as $guildID) {
        if(!is_numeric($guildID)){
            continue;
        }

        $statement = $pdo->prepare("SELECT * FROM `roomGuild` WHERE `guild`=:id");
        $statement->bindParam(':id', $guildID, PDO::PARAM_INT);
        $statement->execute();

        while($row = $statement->fetch(PDO::FETCH_ASSOC)){
            $name = $row['name'];
            $id = $row['id'];

            $returnObj = new StdClass();
            $returnObj->name = $name;
            $returnObj->id = $id;

            array_push($list, $returnObj);
        }
    }

    echo json_encode($list);
}
?>