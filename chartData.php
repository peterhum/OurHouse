<?php

//	db connection
$conn = mysqli_connect("OurHouse", "website", "markit9data", "OurHouse");
// Check connection
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

if ($_GET["type"] == "all" or $_GET["type"] == ""){
	$sql = "SELECT DATE_FORMAT(observedDateTime,'%e-%m %H:%i') observed, 
				   CASE sensorId WHEN 3 THEN value * 100 WHEN 15 then value * 100 ELSE value END value
			  FROM Observations 
			  WHERE sensorId = " . $_GET["sensorid"] . " " . 
			 "AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY) 
		     ORDER BY observedDateTime";
}elseif ($_GET["type"] == "avgbyhour"){
		$sql = "SELECT CONCAT(DATE_FORMAT(observedDateTime,'%e-%m'), ' ', CONVERT(HOUR(observedDateTime),CHAR(2)), '-00') observed,
					 AVG(CASE sensorId WHEN 3 THEN value * 100 WHEN 15 then value * 100 ELSE value END) value
				  FROM Observations 
				  WHERE sensorId = " . $_GET["sensorid"] . " " . 
					"AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY) 
				  GROUP BY DATE(observedDateTime), HOUR(observedDateTime)        
				  ORDER BY observedDateTime;";
}elseif ($_GET["type"] == "avgbyday"){
		$sql = "SELECT DATE_FORMAT(observedDateTime,'%e-%m') observed,
					 AVG(CASE sensorId WHEN 3 THEN value * 100 WHEN 15 then value * 100 ELSE value END) value
				  FROM Observations 
				  WHERE sensorId = " . $_GET["sensorid"] . " " . 
					"AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY) 
				  GROUP BY DATE(observedDateTime)
				  ORDER BY observedDateTime;";
}elseif ($_GET["type"] == "totalbyhour"){
		$sql = "SELECT CONCAT(DATE_FORMAT(observedDateTime,'%e-%m'), ' ', CONVERT(HOUR(observedDateTime),CHAR(2)), '-00') observed,
					 SUM(CASE sensorId WHEN 3 THEN value * 100 WHEN 15 then value * 100 ELSE value END) value
				  FROM Observations 
				  WHERE sensorId = " . $_GET["sensorid"] . " " . 
					"AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY) 
				  GROUP BY DATE(observedDateTime), HOUR(observedDateTime)        
				  ORDER BY observedDateTime;";
}elseif ($_GET["type"] == "totalbyday"){
		$sql = "SELECT DATE_FORMAT(observedDateTime,'%e-%m') observed,
					 SUM(CASE sensorId WHEN 3 THEN value * 100 WHEN 15 then value * 100 ELSE value END) value
				  FROM Observations 
				  WHERE sensorId = " . $_GET["sensorid"] . " " . 
					"AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY) 
				  GROUP BY DATE(observedDateTime)
				  ORDER BY observedDateTime;";
}
$rs = $conn->query($sql);

//initialize the array to store the processed data
$jsonArray = array();
//check if there is any data returned by the SQL Query
if ($rs->num_rows > 0) {
	//Converting the results into an associative array
	while($row = $rs->fetch_assoc()) {
		$jsonArrayItem = array();
      $jsonArrayItem['label'] = $row['observed'];
		$jsonArrayItem['value'] = $row['value'];
		//append the above created object into the main array.
      array_push($jsonArray, $jsonArrayItem);
	}
}


//Closing the connection to DB
$conn->close();

//set the response content type as JSON
header('Content-type: application/json');
//output the return value of json encode using the echo function. 
echo json_encode($jsonArray);

?>
