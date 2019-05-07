<!-- 	Class:       ECE 4970W - Electrical and Computer Engineering Capstone
		Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
		Written by:  Alex Flynn
-->
<!-- This script shows all of the sensor data from the table in the database in an html table-->
<html>
	<head>
		<!-- Configure style elements -->
		<style>
			ul {
			  list-style-type: none;
			  margin: 0;
			  padding: 0;
			  overflow: hidden;
			  background-color: #333;
			}

			li {
			  float: left;
			}
			li a {
			  display: block;
			  color: white;
			  text-align: center;
			  padding: 14px 16px;
			  text-decoration: none;
			}
			li a:hover {
			  background-color: #111;
			}
		</style>
	</head>
	Table Display of All Sensor Data
	<body>
		<!-- Create menu bar -->
		<ul>
		  <li><a href="table_display.php">Table</a></li>
		  <li><a href="general.html">General</a></li>
		  <li><a href="upper_leg.html">Upper Leg</a></li>
		  <li><a href="lower_leg.html">Lower Leg</a></li>
		</ul>
		<?php

			
			// This basically goes back into the database to get the table name ($tname)
			// using the index that was posted from the index.php page
			function get_table($conn,$db){
				// retrieve the status from the /tmp/db_status.txt file
				// 0 = new table being read from - need to get table name
				// 1 = still on same table - table name is saved in the /tmp/db_temp.txt file
				$file = "/tmp/db_status.txt";
				$f = fopen($file,'r');
				$status = fgets($f);
				fclose($f);
				
				// retrieve the name persons name being searched for from the /tmp/db_status.txt file
				$file = "/tmp/db_name_id.txt";
				$f = fopen($file,'r');
				$subPNAME = fgets($f);
				fclose($f);
				
				
				// if status = 0 (need to determine table name using the posted index)
				if($status == '0'){
					// query database for all tables
					$sql = "SHOW TABLES FROM $db";
					$result = $conn->query($sql);
					if (!$result) {
						echo "Error";
					}
					$count = 1;
					$tname = "";
					// get posted index from index.php page
					$selection = $_POST["selection"];
					
					// for every table in reult
					while($row = mysqli_fetch_row($result)){
						// if the name being searched for (entered on the search.html page) is
						// in the table name, add table name to table
						if(strpos($row[0], $subPNAME) !== false){
							// if posted index equals index of current table
							if ($selection == $count){
								// save table name
								$tname = $row[0];
								// write table name to /tmp/db_tmp.txt file
								$file = "/tmp/db_tmp.txt";
								$f = fopen($file,'w');
								fwrite($f, $tname);
								fclose($f);
							}
							$count+=1;
						}
					}
					
					// free query result
					mysqli_free_result($result);
					
					// since the table name is now saved in the /tmp/db_tmp.txt file
					// the status can now be changed to 1 in the /tmp/db_status.txt file
					$file = "/tmp/db_status.txt";
					$f = fopen($file,'w');
					fwrite($f, '1');
					fclose($f);
				// otherwise, if the current table name has already been written to the /tmp/db_tmp.txt file
				// (this occurs if the user were to use the menu bar to go to a different page and then return
				//  back to this page - the index is not reposted)
				// then just read the table name from the /tmp/db_tmp.txt file
				}else{
					// read table name from /tmp/db_tmp.txt file
					$file = "/tmp/db_tmp.txt";
					$f = fopen($file,'r');
					$tname = fgets($f);
					fclose($f);
				}
				// return table name
				return $tname;
			}
		?>
		<!-- This button allows the user to go back to the index.php page (Note: status is reset in the /tmp/db_status.txt file
			within the index.php page -->
		<button onclick="location.href='index.php'" type="button">Back to Sessions</button>
		<?php
			// connect to database
			$servername = "127.0.0.1";
			$dbuser = "root";
			$dbpass = "";
			$db = "brace_db";
			$conn1 = new mysqli($servername,$dbuser,$dbpass,$db) or die("Connection Failed: %s\n". $conn->error);
			
			// get table name and display
			$tname = get_table($conn1,$db);
			echo "<p><b>TABLE: ".$tname."</b></p>";
			echo "<p></p>";

			// below grabs the table data and puts it into an html table
			$sql = "SELECT * FROM $tname";
			$result = $conn1->query($sql);
			$_SESSION['result'] = $result;
			if(!$result) {
				echo "Error";
			}
			echo "<table border = 1>";
			echo "<tr><td><b>SAMPLE</b></td><td><b>DATE TIME</b></td><td><b>YAW1</b></td><td><b>PITCH1</b></td><td><b>ROLL1</b></td><td><b>YAW2</b></td><td><b>PITCH2</b></td><td><b>ROLL2</b></td><td><b>DELTA ROLL</b></td><td><b>Force</b></td></tr></b>";
			while($row2 = $result->fetch_row()){
				echo "<tr>";
				echo "<td>";
				echo $row2[0];
				echo "</td>";
				echo "<td>";
				echo $row2[1];
				echo "</td>";
				echo "<td>";
				echo $row2[2];
				echo "</td>";
				echo "<td>";
				echo $row2[3];
				echo "</td>";
				echo "<td>";
				echo $row2[4];
				echo "</td>";
				echo "<td>";
				echo $row2[5];
				echo "</td>";
				echo "<td>";
				echo $row2[6];
				echo "</td>";
				echo "<td>";
				echo $row2[7];
				echo "</td>";
				echo "<td>";
				echo $row2[8];
				echo "</td>";
				echo "<td>";
				echo $row2[9];
				echo "</td>";
				echo "</tr>";
			}
			echo "</table"
		?>
	</body>
</html>
