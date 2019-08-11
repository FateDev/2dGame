<?php
$error = "";
if(isset($_GET['error'])){
	if($_GET['error'] == "true"){
		$error = "<div class='float-contain-error'>There was an error.</div>";
	}else if($_GET['error'] == "invalidEmail"){
		$error = "<div class='float-contain-error'>The email was invalid.</div>";
	}else if($_GET['error'] == "passwordError"){
		$error = "<div class='float-contain-error'>The password was invalid.</div>";
	}else if($_GET['error'] == "invalidUsername"){
		$error = "<div class='float-contain-error'>The username was invalid.</div>";
	}else if($_GET['error'] == "emailInUse"){
		$error = "<div class='float-contain-error'>The email is already in use.</div>";
	}else if($_GET['error'] == "usernameInUse"){
		$error = "<div class='float-contain-error'>The username is already in use.</div>";
	}
}
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

		.float-contain-error{
			text-align:center;
			padding:30px;
			margin:30px auto;
			background-color:rgba(130,30,30,0.7);
			color:#fff;
			max-width:900px;
		}

		h1{
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
			background-color:rgba(255, 255, 255, 0.7) !important;
		}

		input[type='submit']:active{
			background-color:rgba(255, 255, 255, 0.5) !important;
		}
	</style>
	<body>
		<div class='float-contain'>
			<h1>Game</h1>
			You can sign up for the game on this page, or go to the ones below for whatever you need.
			<br>
			<form action="settings" method="GET" autocomplete="no" style="display: inline-block;">
				<input type="submit" value="Settings" style="color: white;background-color: #3498db;width: 100px;min-width: 0;">
			</form>
			<form action="help" method="GET" autocomplete="no" style="display: inline-block;">
				<input type="submit" value="Help" style="background-color: #3498db;color: white;width: 100px;min-width: 0;">
			</form>
		</div>
		<div class='float-contain'>
			<form action='signup.php' method="POST">
				<input placeholder='Username' name='username' required>
				<input type='email' placeholder='Email' name='email' required>
				<input type='password' autocomplete="new-password" placeholder='Password' name='password' required>
				<input type='password' autocomplete="new-password" placeholder='Password again' name='passwordAgain' required>
				<input type='submit'>
			</form>
		</div>
		<?php echo $error; ?>
	</body>
</html>
