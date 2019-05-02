<!-- 	Class:       ECE 4970W - Electrical and Computer Engineering Capstone
		Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
		Written by:  Alex Flynn
		
		This script was developed following the tutorial at:
			https://www.amcharts.com/docs/v3/tutorials/using-data-loader-to-connect-charts-to-mysql-data-base/
-->
<!-- This script retrieves the sensor data from the database -->
<?php
	// read saved table name /tmp/db_tmp.txt file
	$file = "/tmp/db_tmp.txt";
	$f = fopen($file,'r');
	$tname = fgets($f);
	fclose($f);
	
	# connect to database
	$link = mysql_connect( '127.0.0.1', 'root', '' );
	if ( !$link ) {
	  die( 'Could not connect: ' . mysql_error() );
	}
	
	// Select the brace database
	$db = mysql_select_db( 'brace_db', $link );
	if ( !$db ) {
	  die ( 'Error selecting database : ' . mysql_error() );
	}

	// Query the database for the sensor data
	$query = "
	  SELECT * 
	  FROM $tname";
	$result = mysql_query( $query );

	// If query not a success, die
	if ( !$result ) {
	  $message  = 'Invalid query: ' . mysql_error() . "\n";
	  $message .= 'Whole query: ' . $query;
	  die( $message );
	}

	// for each tuple of result, convert to JSON format
	$prefix = '';
	echo "[\n";
	while ( $row = mysql_fetch_assoc( $result ) ) {
	  echo $prefix . " {\n";
	  echo '  "Sample": "' . $row['Sample'] . '",' . "\n";
	  echo '  "DT": "' . $row['DT'] . '",' . "\n";
	  echo '  "YAW": ' . $row['YAW'] . ',' . "\n";
	  echo '  "PITCH": ' . $row['PITCH'] . ',' . "\n";
	  echo '  "ROLL": ' . $row['ROLL'] . ',' . "\n";
	  echo '  "YAWB": ' . $row['YAWB'] . ',' . "\n";
	  echo '  "PITCHB": ' . $row['PITCHB'] . ',' . "\n";
	  echo '  "ROLLB": ' . $row['ROLLB'] . ',' . "\n";
	  echo '  "D_ROLL": ' . $row['D_ROLL'] . ',' . "\n";
	  echo '  "f": ' . $row['f'] . ',' . "\n";
	  echo " }";
	  $prefix = ",\n";
	}
	echo "\n]";
	
	// Close the connection
	mysql_close($link);
?>