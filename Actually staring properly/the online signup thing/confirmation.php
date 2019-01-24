<?php
if(!isset($_GET['username']) || !isset($_GET['email'])){
    header('Location: index.php?error=true');
}

$handle = fopen("data.csv", "r");

$allLines = "";

$confirmed = false;

if ($handle) {
    while (($line = fgets($handle)) !== false) {
        if(\strpos($line, $_GET['email']) !== false){
            if(\strpos($line, "unconfirmed") !== false){
                $line = str_replace("unconfirmed","confirmed",$line);
                $confirmed = true;
            }
        }
        
        $allLines .= $line;
    }

    fclose($handle);
}

if(!$confirmed){
    header('Location: index.php?error=true');
}else{
    file_put_contents("data.csv", $allLines);
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

		h3{
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
			<h3>Confirmed, now you can login in the client.</h3>
		</div>
	</body>
</html>