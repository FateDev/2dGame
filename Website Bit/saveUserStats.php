<?php
if($_SERVER['REMOTE_ADDR'] == "82.40.114.214" || $_SERVER['REMOTE_ADDR'] == "localhost" || $_SERVER['REMOTE_ADDR'] == "127.0.0.1"){
    require 'info.php';

    $id = $_GET['id'];
    $mp = $_GET['mp'];
    $max_mp = $_GET['max_mp'];
    $hp = $_GET['hp'];
    $max_hp = $_GET['max_hp'];

    $pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

    $statement = "UPDATE usersVerified SET mp=?, max_mp=?, hp=?, max_hp=? WHERE UserID=?";
    $pdo->prepare($statement)->execute([$mp, $max_mp, $hp, $max_hp, $id]);
}
?>
