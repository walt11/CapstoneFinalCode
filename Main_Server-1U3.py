# Class:       ECE 4970W - Electrical and Computer Engineering Capstone
# Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
# Written by:  John Walter & Alex Flynn

# Requires non-standard libraries: Pyseral and MySQL Connector
# This program was written and tested using Blender's included python interpreter

import serial
import serial.tools.list_ports
import time
import os
import mysql.connector
import datetime
import thread as thread

compress_level = 0;
pipe_open = 0			# variable used to know if pipe to Blender script is open or not
pipe_name = 'pipe'		# name of named pipe to communicate with Blender script
i=0
pipeout=0
first=0

# establish connection to MySQL database
db = mysql.connector.connect(
 	host="localhost",
 	user="root",
	passwd="",
 	database="brace_db"
)
# create database cursor
mc = db.cursor()

# This function is called within a new thread.
# It's purpose is to continuously try and open the namaed pipe to the Blender script.
# This allows this program to function without the need of the Blender script running also. 
def fT(x):
	# flag if pipe is opened or closed
	global pipe_open
	# the pipe variable
	global pipeout
	# while the pipe is not open, try to open
	while(not pipe_open):
		try:
			pipeout = os.open(pipe_name, os.O_WRONLY)
			# once the pipe is successfully opened, this thread is killed
			pipe_open = 1
		except:
			pass
	time.sleep(2)

# This function is called within another thread.
# Its purpose if to monitor for user input (a key press)
# during the main loop in the execute() function.
# in_list is the condition for the loop, so while it is empty
# the loop continues. However, onece the user presses a key
# a value gets added to the list which breaks the loop. 
def break_loop_thread(in_list):
	# wait for input
	raw_input()
	# once input, append to list
	in_list.append(True) 

# This function writes values to the named pipe.
# This is for piping sensor data out to the Blender script.
def writeToPipe(toWrite):
    global pipe_open
    global pipeout
    # try to write value to pipe
    try:
        os.write(pipeout, str(toWrite)+"\n")
        pipe_open = 0
    # if exception thrown
    except Exception as e:
    	# if the error is a broken pipe
        if("Broken pipe" in str(e)):
        	# if the pipe is closed
            if(pipe_open == 0):
                print("Pipe Closed")
                pipe_open = 1
        else:
            print(str(e))
    return

# This function writes the sensor values to the database
# This function was written by Alex Flynn
def writeToDatabase(q,table,cursor,db):
	# get current time 
    now = datetime.datetime.now()
    # insert into table the values from the read packet
    command="INSERT INTO "+table+" (Sample,DT,YAW,YAWB,PITCH,PITCHB,ROLL,ROLLB,D_ROLL,f) VALUES(NULL,%s,%s,%s,%s,%s,%s,%s,%s,%s)"
    val = (now,q[9],q[8],q[11],q[10],q[13],q[12],q[14],q[15])
    cursor.execute(command,val)
    # commit changes to database
    db.commit()
    return

# This function is the main function that reads sensor values from the serial connection
# and calls the above functions to write to the database and pipe.
def execute(name):
	global first
	# if this is not the first execution of this function, tell the arduino to reset 
	# and flush the serial buffer
	if(first != 0):
		serialRead.setDTR(False)
		time.sleep(1)
		serialRead.flushInput()
		serialRead.setDTR(True)
	# if it is the first execution, flush the serial buffer
	else:
		first=1
		serialRead.flushInput()

	
	# this thread is used to allow the user to press any key to break the continuous loop below
	in_list = []
	thread.start_new_thread(break_loop_thread, (in_list,))
	print("Sending compression level...")
	# send the user inputted compression level to the arduino
	serialRead.write(str(compress_level).encode('utf-8'))
	print("Starting execution...")
	# tell the arduino to start execution by sending it 'S'
	serialRead.write(str("S").encode('utf-8'))

	start = 1
	# while the user does not press a key
	while not in_list:
		# wait for new serial data
		while(serialRead.inWaiting()==0):
			pass
		# when new data available, read packet
		packet = serialRead.readline()
		# decode packet
		packet = packet.decode('utf-8')
		print(packet)
		q=[]
		# this if statement ignores the very first packet becuase
		# it is not an actual packet of complete data
		if(start != 1):
			# split the packet at commas
			for y in packet.split(","):
				# store each value in an array
				q.append(float(y.strip()))
			# write the sensor data to the database
			writeToDatabase(q,name,mc,db)
			# write the sensor data to the named pipe
			writeToPipe(packet)
		start += 1

# This function scans through the COM ports looking for the serial over Bluetooth
# connection to the Arduino and returns the respective COM port.
def findArduinoCOM():
	ports = []
	# for every active COM port
	for p in list(serial.tools.list_ports.comports()):
		# if name contains the Arduino's bluetooth module
		if("HB-05" in tuple(p)[1]):
			print("[#] Found Arduino on: "+tuple(p)[0])
			return tuple(p)[0]
	print("[!] Did not find a connected Arduino")
	return -1;

# setup serial connection to bluetooth 
try:
	# find the COM port of the Arduino
	com = findArduinoCOM()
	# if not found
	if(com == -1):
		print("[!] Error finding Arduino's COM port")
		exit()
	# setup serial connection
	serialRead = serial.Serial(com,9600)
	print("Connected to serial port.")
# if not able to setup serial connection
except Exception as e:
	print("[!] Error setting up serial connection")
	print(str(e))
	exit()

# create thread to keep named pipe open
thread.start_new_thread(fT, (i,))

# main loop
while(1):
	i=0
	# every instance, close and reopen serial connection
	serialRead.close()
	serialRead.open()
	# prompt user with menu
	select = int(raw_input("Select:\n\t(1) New Session\n\t(2) View Sessions\n\t(3) Delete Session\n\t(0) Quit\n> "))
	# until they enter a correct choice
	while(select > 3 or select < 0):
		print("Bad selection. Try again.")
		select = int(raw_input("Select:\n\t(1) New Session\n\t(2) View Sessions\n\t(3) Delete Session\n\t(0) Quit\n> "))
	# if choice equal to New Session
	if(select == 1):
		# ask for user's first and last name, replace space with '-'
		name = raw_input("Enter user's name (FIRST LAST): ").replace(" ", "_")
		# ask user for compression level: 0 = 100%, 1=10%, 2=20%...
		compress_level = raw_input("Enter compression level (1-9,0): ")
		# until they enter a correct compression level
		while(int(compress_level) > 9 or int(compress_level) < 0):
			print("Error. Please enter a compression level between 1-9, or 0 for max")
			compress_level = input("Enter compression level (1-9,0): ")
		# get current date
		now = datetime.datetime.now()
		# create table name consiting of user's name+current date+compression level
		base = name+"_"+str(now.strftime("%b"))+"_"+str(now.year)+"_C"+compress_level
		# convert table name to uppercase 
		base = base.upper()
		# This loop continuously tries to create table in database, if unsuccessful then there already exists a table with the same name as the one
		# trying to be created. Therefore, a counter appended at the end of the table name is incremented until a table name is found that doesn't
		# exists. This would typically occur if a user was to do more than one session of wearing the brace on the same day.
		while(1):
			try:
				# try to create table in database
				command = "CREATE TABLE "+base+"_"+str(i)+" (Sample INT AUTO_INCREMENT PRIMARY KEY, DT DATETIME(6), YAW float(5,2), PITCH float(5,2), ROLL float(5,2), YAWB float(5,2), PITCHB float(5,2), ROLLB float(5,2), D_ROLL float(5,2), f INT)"
				mc.execute(command)
				print("Created session "+base+"_"+str(i)+" in database.")
				break;
			except Exception as e:
				# if table already exists, increment i 
				if("already exists" in str(e)):
					i+=1
		# call the execute() function to begin interaction with brace
		execute(base+"_"+str(i))
		# once user exits the execution script, close serail connection
		serialRead.close()
		print("Execution stopped.")
	# if choice equal to View Sessions
	elif(select == 2):
		tables=[]
		# query database for all tables
		mc.execute("SHOW TABLES")
		# for every table in database
		for (table_name,) in mc:
			# add table to array and print table name (session name)
			tables.append(table_name)
			print("\t"+table_name)
		# otherwise if the table is empty, display as so
		if(len(tables) == 0):
			print("No sessions found.")
	# if choice equal to Delete Session
	elif(select == 3):
		print("Select session to delete: ")
		x=1
		tables=[]
		# query database for all tables
		mc.execute("SHOW TABLES")
		# display all table names
		for (table_name,) in mc:
			tables.append(table_name)
			print("\t("+str(x)+") "+table_name)
			x+=1
		if(len(tables) == 0):
			print("No sessions found.")
		else:
			# ask the user to pick which table they want to delete
			choice = int(input("> "))
			# loop until they enter an actual table
			while(choice > len(tables) or choice < 1):
				print("Bad choice. Try again.")
				choice = int(input("> "))
			# query the database to drop the selected table
			mc.execute("drop table "+tables[choice-1])
			print("Session deleted.")
	# if choice equal to Quit
	else:
		# ensure serial connection is closed
		serialRead.close()
		# exit program
		exit()







