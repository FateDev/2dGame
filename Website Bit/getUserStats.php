<?php
if($_SERVER['REMOTE_ADDR'] == "82.40.114.214" || $_SERVER['REMOTE_ADDR'] == "localhost" || $_SERVER['REMOTE_ADDR'] == "127.0.0.1"){
    require 'info.php';
    $id = $_GET['id'];

    $pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

    $statement = $pdo->prepare("SELECT mp, hp, max_mp, max_hp, balance FROM usersVerified WHERE UserID=:id");
    $statement->bindParam(':id', $id, PDO::PARAM_INT);
    $statement->execute();

    $row = $statement->fetch(PDO::FETCH_ASSOC);
    $balance = $row['balance'];
    $mp = $row['mp'];
    $hp = $row['hp'];
    $max_mp = $row['max_mp'];
    $max_hp = $row['max_hp'];
    $stats = array("balance"=>floatval($balance), "mp"=>floatval($mp), "hp"=>floatval($hp), "max_mp"=>floatval($max_mp), "max_hp"=>floatval($max_hp));
    echo json_encode($stats); //gives values as floats rather than strings
}
?>
