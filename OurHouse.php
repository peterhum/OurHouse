<!DOCTYPE html>
<html>

<?php
	//	sort out wether it's a browser or mobile
	$iPhone = "N";
	if(strpos($_SERVER['HTTP_USER_AGENT'], "iPhone") or strpos($_SERVER['HTTP_USER_AGENT'], "Android")){
		$iPhone = "Y";
	}

	//	Display errors
	error_reporting(E_ALL);
	ini_set('display_errors', '1');

	// Create connection
	$conn = mysqli_connect("OurHouse", "website", "markit9data", "OurHouse");
	// Check connection
	if (!$conn) {
		 die("Connection failed: " . mysqli_connect_error());
	}
	$oldest = strtotime("2030-12-31 23:59:59");

function sensorLine($sensorId, $conn, $oldest){

	//	get some stuff about the sensor
	if ($sensorId == 13){	//	rainfall
		$sql = "SELECT s.description,
			    		   s.units,
			            s.displayDecimalPlaces,
							SUM(o.value) value, 
							MAX(o.observedDateTime) observedDateTime 
				  FROM Sensors s
					INNER JOIN Observations o on s.id = o.sensorId
				  WHERE s.id = 13 AND DATE(o.observedDateTime) = CURDATE()";
	}else{
		if ($sensorId == 3 or $sensorId == 15){
			$sql = "SELECT s.description,
						 s.units,
						 s.displayDecimalPlaces,
						 o.value * 100 value,
						 o.observedDateTime 
					FROM Sensors s
						INNER JOIN Observations o on s.id = o.sensorId
					WHERE s.Id = " . $sensorId . " " . 
					"ORDER BY o.observedDateTime DESC LIMIT 1";
		}else{
			$sql = "SELECT s.description,
						 s.units,
						 s.displayDecimalPlaces,
						 o.value,
						 o.observedDateTime 
					FROM Sensors s
						INNER JOIN Observations o on s.id = o.sensorId
					WHERE s.Id = " . $sensorId . " " . 
					"ORDER BY o.observedDateTime DESC LIMIT 1";
		}
	}
	$rs = $conn->query($sql);	
	if ($row = $rs->fetch_assoc()){
		echo "<tr>";
		echo "<td valign='bottom'><a href='fusionChart.php?sensorid=" . $sensorId . "&days=1'>" .
			$row["description"] . "</a></td>";
		echo "<td align='right' valign='bottom'>" . number_format($row["value"],$row["displayDecimalPlaces"]) . 
				"</td><td>" . $row["units"] ."</td></tr>";
	}

	if (strtotime($row['observedDateTime']) < $oldest){
		return strtotime($row['observedDateTime']);
	}else{
		return $oldest;
	}
	$rs->close;
}

?>

<head>
	<title>Our House</title>
	<link rel="icon" type="image/png" href="OurHouse.ico">
	<link href="OurHouse120.png" rel="apple-touch-icon" sizes="120x120">
	<link href="OurHouse152.png" rel="apple-touch-icon" sizes="152x152">
	<style>
		<?php
			if ($iPhone=="Y") 
			{
				echo "h1 {font-family:arial; font-size:450%;}";
				echo "p  {font-family:arial; font-size:200%;}";
				echo "ps {font-family:arial; font-size:150%;}";
			}else{
				echo "h1 {font-family:arial; font-size:150%;}";
				echo "p  {font-family:arial; font-size:100%;}";
				echo "ps {font-family:arial; font-size:75%;}";
			}
		?>
	</style>
</head>

<body>

<h1>Our House</h1>
<?php
	if ($iPhone=="Y") 
	{
		echo "<table style='font-family:arial; font-size:300%; align=bottom' width='900px' >";
	}else{
		echo "<table style='font-family:arial; font-size:100%; align=bottom' width='300px' >";
	}

	$oldest = sensorLine(1,$conn,$oldest);		//	outside temp
	$oldest = sensorLine(9,$conn,$oldest);		// family room temp
	$oldest = sensorLine(3,$conn,$oldest);		// outside humidity
	$oldest = sensorLine(2,$conn,$oldest);		//	pressure
	$oldest = sensorLine(10,$conn,$oldest);	//	wind speed average
	$oldest = sensorLine(11,$conn,$oldest);	//	wind speed gust
	$oldest = sensorLine(12,$conn,$oldest);	//	wind direction
	$oldest = sensorLine(13,$conn,$oldest);	//	rainfall

	echo "<tr><td>&nbsp</td><td></td><td></td></tr>";

	$oldest = sensorLine(4,$conn,$oldest);		//	volts
	$oldest = sensorLine(5,$conn,$oldest);		//	power consumption

	echo "<tr><td>&nbsp</td><td></td><td></td></tr>";

	$oldest = sensorLine(7,$conn,$oldest);		//	fish tank temp
	$oldest = sensorLine(14,$conn,$oldest);	//	3rd bedroom temp
	$oldest = sensorLine(15,$conn,$oldest);	//	3rd bedroom humidity

	echo "</table>";
	echo "<p>&nbsp;</p>";
	echo "<ps>Oldest: " . date('d/m/Y H:i:s',$oldest) . "</ps>";
	
?>

<ps>&nbsp;</ps>
<p><a href='controls.php'>Controls</a></p>
</body>

<?php
//	Close
$conn->close();
?> 

</html>
