<?php
require 'info.php';

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);
$output = "";

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
?>