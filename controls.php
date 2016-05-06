<!DOCTYPE html>
<html>

<?php
	//	Display errors
	error_reporting(E_ALL);
	ini_set('display_errors', '1');

	//	check out whether we are a browser or mobile
	$iPhone = "N";
	if(strpos($_SERVER['HTTP_USER_AGENT'], "iPhone") or strpos($_SERVER['HTTP_USER_AGENT'], "Android")){
		$iPhone = "Y";
	}

	//	if we've come back into here with a deviceId and setStateTo then call the set state 
	//	function
	if (isset($_GET['deviceId'])){
		setState($_GET["deviceId"], $_GET["setStateTo"]);
	}

	// Create connection
	$conn = mysqli_connect("localhost", "website", "markit9data", "OurHouse");
	if (!$conn) {
		 die("Connection failed: " . mysqli_connect_error());
	}
?>

<head>
<style>

<?php
if ($iPhone=="Y"){
	echo "h1 {font-family:arial; font-size:450%;}";
}else{
	echo "h1 {font-family:arial; font-size:150%;}";
}
?>
	p  {font-family:arial; font-size:100%;}
</style>
</head>

<body>

<h1>Our House</h1>
<?php if ($iPhone=="Y") 
{
	echo "<table style='font-family:arial; font-size:300%; align=bottom' width='900px' >";
}else{
	echo "<table style='font-family:arial; font-size:100%; align=bottom' width='300px' >";
}
	$rs = $conn->query("SELECT id, description, controlTypeid FROM Devices ORDER BY id");	
	while($row = $rs->fetch_assoc()){
		echo "<tr>";
		echo "<td valign='center'>" . $row["description"] . "</a></td>";
		if (deviceState($row["id"]) == '1'){
			echo "<td valign='center'><a href='controls.php?deviceId=" . $row["id"] . "&setStateTo=0'><img border='0' src='switchOn.png'></a></td>";
		}
		else{
			echo "<td valign='center'><a href='controls.php?deviceId=" . $row["id"] . "&setStateTo=1'><img border='0' src='switchOff.png'></a></td>";
		}
		
		echo "</tr>";
	}
?>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td>&nbsp</td></tr>
<tr><td><a href='OurHouse.php'>Back</a></td></tr>
</table>

<p>&nbsp;</p>
</body>

<?php
//	Close
$conn->close();
?> 

</html>



<?php
function deviceState($deviceId)
{
	$sock = socket_create(AF_INET, SOCK_STREAM, 0);
	if(!socket_connect($sock , '127.0.0.1' , 6675)){
		return "Could not connect";
	}
//	socket_recv ( $sock , $buf , 500 , MSG_PEEK );
//	socket_recv ( $sock , $buf , strlen($buf) , MSG_WAITALL);
	$message = sprintf("QS%03d",$deviceId);
	if( ! socket_send ( $sock , $message , strlen($message) , 0)){
		socket_close($sock);
		return "Count not send";
	}
	while (TRUE){
		$rtn = socket_recv ( $sock , $buf , 500 , 0 );
		if ($rtn > 0){
			break;
		}elseif ($rtn === 0){
			socket_close($sock);
			return "Socket error";
		}
	}
	socket_close($sock);
   return $buf[8];		//	we get the CN from the connect plus the RS0021 (device 2 state 1)
}

function setState($deviceId, $setStateTo)
{
	//	connect
	$sock = socket_create(AF_INET, SOCK_STREAM, 0);
	if(!socket_connect($sock , '127.0.0.1' , 6675)){
		perror("Could not connect");
	}
//	socket_recv ( $sock , $buf , 500 , MSG_PEEK );
//	socket_recv ( $sock , $buf , strlen($buf) , MSG_WAITALL);
	//	send change state msg
	$message = sprintf("ES%03d%d",$deviceId,$setStateTo);
	if( ! socket_send ( $sock , $message , strlen($message) , 0)){
		perror("Count not send");
	}
	while (TRUE){
		$rtn = socket_recv ( $sock , $buf , 500 , 0 );
		if ($rtn > 0){
			break;
		}elseif ($rtn === 0){
			socket_close($sock);
			return "Socket error";
		}
	}
	if (substr($buf,3,2) != "WR"){
		socket_close($sock);
		return "Bad return code on ES";
	}

	usleep(300000);	//	give it 0.3 seconds to get through
	//	then check 20 times with a 0.1 second sleep for the correct state
	for ($i = 1; $i <= 20; $i++) {
		//	send a QS	
		$message = sprintf("QS%03d",$deviceId);
		if( ! socket_send ( $sock , $message , strlen($message) , 0)){
			socket_close($sock);
			return "Count not send QS";
		}
		//	check the response
		while (TRUE){
			$rtn = socket_recv ( $sock , $buf , 500 , 0 );
			if ($rtn > 0){
				break;
			}elseif ($rtn === 0){
				socket_close($sock);
				return "Socket error";
			}
		}

		if ($buf[5] == $setStateTo){
			return $buf[5];
		}
		usleep(100000);
	}
	return "Time out";
	socket_close($sock);
	
/*	//	check the status
	for ($i = 1; $i <= 20; $i++) {
		if (deviceState($deviceId) == $setStateTo){
			return "OK";
		}
		usleep(100000);
	}
	
	return "Timed out"; */
}

function perror($msg)
{
    $errorcode = socket_last_error();
    $errormsg = socket_strerror($errorcode);
    die("$msg: [$errorcode] $errormsg<BR>");
}
?>

