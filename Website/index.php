<!-- 	Class:       ECE 4970W - Electrical and Computer Engineering Capstone
		Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
		Written by:  Alex Flynn
-->
<!-- This script shows all of the sessions in the database and asks the user to select one-->

<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<title>Capstone</title>
	</head>
	<body>
		<?php
			// reset the status index in the /tmp/db_status.txt file
			// the purpose of the status is to indicate whether or not
			// the current selected table name has been saved to the /tmp/db_tmp.txt file
			$file = "/tmp/db_status.txt";
			$f = fopen($file,'w');
			fwrite($f, '0');
			fclose($f);
			
			// connect to database
			$servername = "127.0.0.1";
			$dbuser = "root";
			$dbpass = "";
			$db = "brace_db";
			$conn = new mysqli($servername,$dbuser,$dbpass,$db) or die("Connection Failed: %s\n". $conn->error);
			
			// query database for all table names
			$sql = "SHOW TABLES FROM $db";
			$result = $conn->query($sql);
			if (!$result) {
				echo "Error";
			}
			
			// save the posted patient name, replace the space with '-'
			// and convert string to uppercase
			$subPNAME = $_POST["patientName"];
			$subPNAME = str_replace(" ","_",$subPNAME);
			$subPNAME = strtoupper($subPNAME);
			// save search name in /tmp/db_name_id.txt
			$file = "/tmp/db_name_id.txt";
			$f = fopen($file,'w');
			fwrite($f, $subPNAME);
			fclose($f);
			
			// for each table name that matches the inputed user, put in html table with a designated index
			// basically shows all tables (sessions) that match the search for name on the search.html page
			echo "<table border=1>";
			echo "<tr><td><b>Index</b></td><td><b>Session Name</b></td></tr>";
			$count = 1;
			// for each table in database
			while($row = mysqli_fetch_row($result)){
				// if the name being searched for (entered on the search.html page) is
				// in the table name, add table name to table
				if(strpos($row[0], $subPNAME) !== false){
					echo "<tr><td>".$count."</td>";
					echo "<td>".$row[0]."</td></tr>";
					$count+=1;
				}
			}
			echo "</table>";
			$conn->close();
		?>
		<p></p>
		
		<!-- ask the user to select a session by entering its respective index -->
		<b1><b>Enter Session Index:</b></b1>
		<!-- this form posts the index to the table_display.php file -->
		<form action = "table_display.php" method = "post">
			<input type="text" name="selection" placeholder="TYPE SESSION INDEX HERE" required >
			<input type="submit" value="FIND">
		</form>
	</body>
</html>
