<?php
if($_SERVER['REMOTE_ADDR'] == "82.40.114.214" || $_SERVER['REMOTE_ADDR'] == "localhost" || $_SERVER['REMOTE_ADDR'] == "127.0.0.1"){
    require 'info.php';

    $id = $_GET['id'];
    $avatar = $_GET['avatar'];

    $pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

    $statement = "UPDATE usersVerified SET avatar=? WHERE UserID=?";
    $pdo->prepare($statement)->execute([$avatar, $id]);
}
?>