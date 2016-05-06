<?php
	//	db connection
	$conn = mysqli_connect("OurHouse", "website", "markit9data", "OurHouse");
	// Check connection
	if (!$conn) {
		 die("Connection failed: " . mysqli_connect_error());
	}
	/*  get some information about the sensor  */
	$rs = $conn->query("SELECT description, units, displayDecimalPlaces FROM Sensors WHERE id = " . $_GET["sensorid"]);	
	$row = $rs->fetch_assoc();
	$sensorDescription = $row["description"];
	$sensorUnits = $row["units"];
	$displayDecimalPlaces = $row["displayDecimalPlaces"];
	$valueMultiplier = 1;
	if ($_GET["sensorid"] == 3 or $_GET["sensorid"] == 15){	
		$valueMultiplier = 100;
	}
	/*  get the minimum data value because I can't get the cursed setAdaptiveYMin to work  */
	$rs = $conn->query("SELECT  min(value) minValue, max(value) maxVal " .
							 "FROM Observations WHERE sensorId = " . $_GET["sensorid"] .
							 " AND observedDateTime > DATE_SUB(now(), INTERVAL " . $_GET["days"] . " DAY)");
	if ($row = $rs->fetch_assoc()){
		$minY = intval($row["minValue"] * $valueMultiplier);
		$maxY = intval($row["maxVal"] * $valueMultiplier)+1;
		$maxY = (intval(($maxY - $minY) / 5) + 1) * 5 + $minY;
	}else{
		$minY = 0;
	}
	$conn->close();
?>


<html>
	<head>
		<title>Our House</title>
		<link rel="icon" type="image/png" href="OurHouse.ico">
		<link href="OurHouse120.png" rel="apple-touch-icon" sizes="120x120">
		<link href="OurHouse152.png" rel="apple-touch-icon" sizes="152x152">
		<style>
			<?php
			//    some browser/mobile scaling issues  
			$iPhone = "N";
			if(strpos($_SERVER['HTTP_USER_AGENT'], "iPhone") or strpos($_SERVER['HTTP_USER_AGENT'], "Android")){
				$iPhone = "Y";
			}
			 if ($iPhone=="Y") 
			{
				echo "h1 {font-family:arial; font-size:450%;}";
				echo "p {font-family:arial; font-size:250%;}";
				$chartFontSize = "24";
			}else{
				echo "h1 {font-family:arial; font-size:150%;}";
				echo "p {font-family:arial; font-size:100%;}";
				$chartFontSize = "12";
			}
			?>
		</style>
	</head>
	<body>
		<h1>Our House</h1>

		<?php echo "<p>" . $sensorDescription . " last " . $_GET["days"] . " day(s) in " . $sensorUnits . ".</p>"; ?>

		<!--   fusion chart stuff  -->
		<div id="chart-container"></div>
		<script src="js/jquery-2.1.4.js"></script>
		<script src="js/fusioncharts.js"></script>
		<script src="js/fusioncharts.charts.js"></script>
		<script src="js/themes/fusioncharts.theme.zune.js"></script>
		<script type="text/javascript">
$(function() {
    $.ajax({

        <?php echo "url: 'http://192.168.1.55/chartData.php?sensorid=" . $_GET["sensorid"] . "&days=" . $_GET["days"] . "&type=all',"; ?>
        type: 'GET',
        success: function(data) {
            chartData = data;
            var chartProperties = {
					 "labelStep": "<?php echo 15 * $_GET["days"]; ?>",
                "showValues": "0",
                "drawAnchors": "0",
                "theme": "zune",
					 "yAxisMinValue": "<?php echo $minY; ?>",
					 "yAxisMaxValue": "<?php echo $maxY; ?>",
					 "yFormatNumberScale": "0",
					 "outCnvBaseFontSize": "<?php echo $chartFontSize; ?>",
					 "adjustDiv": "0",
					 "numDivLines": "4"
            };

            apiChart = new FusionCharts({
                type: 'line',
                renderAt: 'chart-container',
                width: '950',
                height: '450',
                dataFormat: 'json',
                dataSource: {
                    "chart": chartProperties,
                    "data": chartData
                }
            });
            apiChart.render();
        }
    });
});
</script>

		<?php
		echo "<p>";
		echo "<a href='fusionChart.php?sensorid=" . $_GET["sensorid"] . "&days=1'>Last 24 hours</a> &nbsp &nbsp";
		echo "<a href='fusionChart.php?sensorid=" . $_GET["sensorid"] . "&days=7'>Last week</a> &nbsp &nbsp";
		echo "<a href='fusionChart.php?sensorid=" . $_GET["sensorid"] . "&days=30'>Last month</a> &nbsp &nbsp";
		echo "<a href='OurHouse.php'>Back</a></p>";
		?>

	</body>
</html>
