<?php
require 'info.php';

if(isset($_POST['username']) && isset($_POST['email']) && isset($_POST['password']) && isset($_POST['passwordAgain'])){
    $username = $_POST['username'];
    $email = $_POST['email'];
    $password = $_POST['password'];
    $passwordAgain = $_POST['passwordAgain'];
}else{
    header('Location: index.php?error=true');
	exit();
}

if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
    header('Location: index.php?error=invalidEmail');
	exit();
}

if(strlen($password) > 20 || strlen($password) < 5 || $password != $passwordAgain){
    header('Location: index.php?error=passwordError');
	exit();
}

if(strlen($username) > 20){
    header('Location: index.php?error=invalidUsername');
	exit();
}

//the above retrieve the post info and validate it

$pdo = new PDO("mysql:host=$host;dbname=$database", $dbUsername, $dbPassword);

$statement = $pdo->prepare("SELECT email FROM usersVerified WHERE email=:email");
$statement->bindParam(':email', $email, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	header('Location: index.php?error=emailInUse');
	exit();
}

$statement = $pdo->prepare("SELECT username FROM usersVerified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	header('Location: index.php?error=usernameInUse');
	exit();
}

$statement = $pdo->prepare("SELECT username FROM usersUnverified WHERE username=:username");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	header('Location: index.php?error=usernameInUse');
	exit();
}

//the above will check if a username or email are already in use in the verified or unverfied tables (not emails in the unverified one)

$statement = $pdo->prepare("SELECT email FROM usersUnverified WHERE email=:email");
$statement->bindParam(':email', $email, PDO::PARAM_STR);
$statement->execute();

if($statement->rowCount() != 0){
	$statement = $pdo->prepare("DELETE FROM usersUnverified WHERE email=:email"); //deletes signup request in unverified table if using the same email as the new request
	$statement->bindParam(':email', $email, PDO::PARAM_STR);
	$statement->execute();
}

$passwordHash = password_hash($password, PASSWORD_DEFAULT); //hashes the password

//the below will add a signup request to the unverfied table
$statement = $pdo->prepare("INSERT INTO usersUnverified (username, pword, email) VALUES (:username, :password, :email)");
$statement->bindParam(':username', $username, PDO::PARAM_STR);
$statement->bindParam(':email', $email, PDO::PARAM_STR);
$statement->bindParam(':password', $passwordHash, PDO::PARAM_STR);
$statement->execute();

//the below will encrypt and urlencode the request email, username and password (not the hashed password)
$encryptEmail = rawurlencode(openssl_encrypt($email, $cipher, $key, $options=0, $iv));
$encryptUsername = rawurlencode(openssl_encrypt($username, $cipher, $key, $options=0, $iv));
$encryptPassword = rawurlencode(openssl_encrypt($password, $cipher, $key, $options=0, $iv));

$to      = $email;
$subject = "Account confirmation";
$message = "To confirm your account, go here:\nerewhon.xyz/game/confirmation.php?a=$encryptUsername&b=$encryptEmail&c=$encryptPassword";
$headers = 'From: admin@erewhon.xyz' . "\r\n" .
    'X-Mailer: PHP/' . phpversion();

mail($to, $subject, $message, $headers); //and a confirmation email will be sent
?>

<!DOCTYPE html>
<html>
	<head>
		<title>Game</title>
		<meta name="description" content="Game">
		<meta property="og:title" content="Game" />
		<meta property="og:url" content="https://www.erewhon.xyz/game/" />
		<meta property="og:description" content="Game">
		<meta property="og:type" content="article" />
		<meta property="og:locale" content="en_GB" />
		<meta property="og:image" content="../images/erewhon.png">
		<link rel="stylesheet" type="text/css" href="../css/main.css">
		<link rel="apple-touch-icon" sizes="57x57" href="../favicons/apple-icon-57x57.png">
		<link rel="apple-touch-icon" sizes="60x60" href="../favicons/apple-icon-60x60.png">
		<link rel="apple-touch-icon" sizes="72x72" href="../favicons/apple-icon-72x72.png">
		<link rel="apple-touch-icon" sizes="76x76" href="../favicons/apple-icon-76x76.png">
		<link rel="apple-touch-icon" sizes="114x114" href="../favicons/apple-icon-114x114.png">
		<link rel="apple-touch-icon" sizes="120x120" href="../favicons/apple-icon-120x120.png">
		<link rel="apple-touch-icon" sizes="144x144" href="../favicons/apple-icon-144x144.png">
		<link rel="apple-touch-icon" sizes="152x152" href="../favicons/apple-icon-152x152.png">
		<link rel="apple-touch-icon" sizes="180x180" href="../favicons/apple-icon-180x180.png">
		<link rel="icon" type="image/png" sizes="192x192"  href="../favicons/android-icon-192x192.png">
		<link rel="icon" type="image/png" sizes="32x32" href="../favicons/favicon-32x32.png">
		<link rel="icon" type="image/png" sizes="96x96" href="../favicons/favicon-96x96.png">
		<link rel="icon" type="image/png" sizes="16x16" href="../favicons/favicon-16x16.png">
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0">
		<link rel="manifest" href="../favicons/manifest.json">
		<meta name="msapplication-TileColor" content="#ffffff">
		<meta name="msapplication-TileImage" content="../favicons/ms-icon-144x144.png">
		<meta name="theme-color" content="#ffffff">
		<link href="https://fonts.googleapis.com/css?family=Lato:300,400" rel="stylesheet">
	</head>
	<style>
		.float-contain{
			text-align:center;
			padding:30px;
			margin:30px auto;
			background-color:rgba(0,0,0,0.7);
			color:#fff;
			max-width:900px;
		}

		h3
			padding:10px;
		}

		input{
			text-align: center;
			min-width: 50%;
			padding:10px;
			border:0;
			border-radius:5px;
			margin:15px auto;
			display:block;
			transition:background-color 0.2s ease;
		}

		input[type='submit']{
			background-color:rgba(255, 255, 255, 0.9);
			padding:15px;
		}

		input[type='submit']:hover{
			background-color:rgba(255, 255, 255, 0.7);
		}

		input[type='submit']:active{
			background-color:rgba(255, 255, 255, 0.5);
		}
	</style>
	<body>
		<div class='float-contain'>
			<h3>Check your spam folders for the confirmation email.</h3>
		</div>
	</body>
</html>
