<?php
require 'verifySession.php';

if($sessionActive == false){
	header('Location: index.php');
	exit();
}

$statement = $pdo->prepare("SELECT settings FROM usersVerified WHERE UserID=:id");
$statement->bindParam(':id', $_SESSION['userGame']['id'], PDO::PARAM_STR);
$statement->execute();

$settings = $statement->fetch(PDO::FETCH_ASSOC);
$settings = json_decode($settings['settings'], true);

$privateMessaging = $settings['privateMessaging'];

if($privateMessaging == true){
	$checked = "checked";
}else{
	$checked = "";
}

$msg = "";
if(isset($_GET['error'])){
	if($_GET['error'] == "incorrectDetails"){
		$msg = "<div class='float-contain-error'>Incorrect details.</div>";
	}else if($_GET['error'] == "passwordError"){
		$msg = "<div class='float-contain-error'>The password was invalid.</div>";
	}else if($_GET['error'] == "passNotMatch"){
		$msg = "<div class='float-contain-error'>The new passwords do not match.</div>";
	}
}else if(isset($_GET['success'])){
	$msg = "<div class='float-contain-success'>The changes have been saved.</div>";
}

?>
<!DOCTYPE html>
<html>
	<head>
		<title>Settings</title>
		<meta name="description" content="Game">
		<meta property="og:title" content="Game" />
		<meta property="og:url" content="https://www.erewhon.xyz/game/" />
		<meta property="og:description" content="Game">
		<meta property="og:type" content="article" />
		<meta property="og:locale" content="en_GB" />
		<meta property="og:image" content="../../images/erewhon.png">
		<link rel="stylesheet" type="text/css" href="../../css/main.css">
		<link rel="apple-touch-icon" sizes="57x57" href="../../favicons/apple-icon-57x57.png">
		<link rel="apple-touch-icon" sizes="60x60" href="../../favicons/apple-icon-60x60.png">
		<link rel="apple-touch-icon" sizes="72x72" href="../../favicons/apple-icon-72x72.png">
		<link rel="apple-touch-icon" sizes="76x76" href="../../favicons/apple-icon-76x76.png">
		<link rel="apple-touch-icon" sizes="114x114" href="../../favicons/apple-icon-114x114.png">
		<link rel="apple-touch-icon" sizes="120x120" href="../../favicons/apple-icon-120x120.png">
		<link rel="apple-touch-icon" sizes="144x144" href="../../favicons/apple-icon-144x144.png">
		<link rel="apple-touch-icon" sizes="152x152" href="../../favicons/apple-icon-152x152.png">
		<link rel="apple-touch-icon" sizes="180x180" href="../../favicons/apple-icon-180x180.png">
		<link rel="icon" type="image/png" sizes="192x192"  href="../../favicons/android-icon-192x192.png">
		<link rel="icon" type="image/png" sizes="32x32" href="../../favicons/favicon-32x32.png">
		<link rel="icon" type="image/png" sizes="96x96" href="../../favicons/favicon-96x96.png">
		<link rel="icon" type="image/png" sizes="16x16" href="../../favicons/favicon-16x16.png">
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0">
		<link rel="manifest" href="../../favicons/manifest.json">
		<meta name="msapplication-TileColor" content="#ffffff">
		<meta name="msapplication-TileImage" content="../../favicons/ms-icon-144x144.png">
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

		.float-contain-success{
			text-align:center;
			padding:30px;
			margin:30px auto;
			background-color:#27ae60;
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
			background-color:#42a5f5;
			padding:15px;
			color:#fff;
		}

		input[type='submit']:hover{
			background-color:rgba(255, 255, 255, 0.7) !important;
			color:#000;
		}

		input[type='submit']:active{
			background-color:rgba(255, 255, 255, 0.5) !important;
			color:#000;
		}
		
		
		
		.onoffswitch {
			margin:20px auto;
			position: relative; width: 40px;
			-webkit-user-select:none; -moz-user-select:none; -ms-user-select: none;
		}
		.onoffswitch-checkbox {
			display: none;
		}
		.onoffswitch-label {
			display: block; overflow: hidden; cursor: pointer;
			height: 9px; padding: 0; line-height: 9px;
			border: 0px solid #FFFFFF; border-radius: 16px;
			background-color: #9E9E9E;
		}
		.onoffswitch-label:before {
			content: "";
			display: block; width: 16px; margin: -3.5px;
			background: #FFFFFF;
			position: absolute; top: 0; bottom: 0;
			right: 27px;
			border-radius: 16px;
			box-shadow: 0 6px 12px 0px #757575;
		}
		.onoffswitch-checkbox:checked + .onoffswitch-label {
			background-color: #42A5F5;
		}
		.onoffswitch-checkbox:checked + .onoffswitch-label, .onoffswitch-checkbox:checked + .onoffswitch-label:before {
		   border-color: #42A5F5;
		}
		.onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-inner {
			margin-left: 0;
		}
		.onoffswitch-checkbox:checked + .onoffswitch-label:before {
			right: 0px; 
			background-color: #2196F3; 
			box-shadow: 3px 6px 18px 0px rgba(0, 0, 0, 0.2);
		}
</style>
	<body>
		<div class='float-contain'>
			<h1>Settings</h1>
			Update your details and privacy settings here.
			<form action='logout.php' method="GET" autocomplete="no">
				<input type='submit' value='Logout' style='background-color: #95a5a6; width: 100px; min-width: 0;'>
			</form>
		</div>
		<div class='float-contain'>
			<form action='saveSettings.php' method="POST" autocomplete="no">
				<input placeholder='Username' name='username'>
				<input type='password' placeholder='Password' name='password' autocomplete="new-password">
				<span>Repeat new passwords below to change your password</span>
				<input type='password' placeholder='New Password' name='newPass' autocomplete="new-password">
				<input type='password' placeholder='New Password Repeat' name='newPassRepeat' autocomplete="new-password">
				<span>Toggle whether you want private messaging enabled</span>
				<div class="onoffswitch">
					<input type="checkbox" name="privateMessaging" class="onoffswitch-checkbox" value="on" id="myonoffswitch" <?php echo $checked; ?>>
					<label class="onoffswitch-label" for="myonoffswitch"></label>
				</div>
				<input type='submit' value='Save Changes'>
			</form>
		</div>
		<?php echo $msg; ?>
	</body>
</html>