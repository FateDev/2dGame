<?php
require 'info.php';

$key = urldecode($_GET['key']);
$userID = $_GET['userID'];
$guildName = $_GET['guildName'];
$joined = $_GET['joined'];

if($clientAuthKey == $key){
    $pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

    $statement = $pdo->prepare("SELECT settings FROM usersVerified WHERE UserID=:id");
    $statement->bindParam(':id', $userID, PDO::PARAM_INT);
    $statement->execute();

    $row = $statement->fetch(PDO::FETCH_ASSOC);

    $settings = json_decode($row['settings']);
    
    $guilds = $settings->guilds;

    $statement = $pdo->prepare("SELECT id FROM `guilds` WHERE `name`=:name");
    $statement->bindParam(':name', $guildName, PDO::PARAM_STR);
    $statement->execute();

    $row = $statement->fetch(PDO::FETCH_ASSOC);
    $guildIDstring = $row['id']."#";

    if($joined == 0){
        $guilds = str_replace($guildIDstring,"", $guilds);
    }else if($joined == 1){
        if(strpos($guilds, $guildIDstring) === false){
            $guilds .= $guildIDstring;
        }
    }

    $settings->guilds = $guilds;
    $settings = json_encode($settings);

    $statement = $pdo->prepare("UPDATE usersVerified SET settings=:settings WHERE UserID=:id");
    $statement->bindParam(':id', $userID, PDO::PARAM_INT);
    $statement->bindParam(':settings', $settings, PDO::PARAM_STR);
    $statement->execute();
}