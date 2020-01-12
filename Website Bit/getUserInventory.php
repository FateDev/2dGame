<?php
if($_SERVER['REMOTE_ADDR'] == "82.40.114.214" || $_SERVER['REMOTE_ADDR'] == "localhost" || $_SERVER['REMOTE_ADDR'] == "127.0.0.1"){
    require 'info.php';

    $id = $_GET['id'];

    $pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

    $statement = $pdo->prepare("SELECT inventory FROM usersVerified WHERE UserID=:id");
    $statement->bindParam(':id', $id, PDO::PARAM_INT);
    $statement->execute();

    $row = $statement->fetch(PDO::FETCH_ASSOC);
    echo $row['inventory'];
}
?>
