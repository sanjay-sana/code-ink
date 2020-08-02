a. Name: Sanjay Sana
b. USC ID: 85818009897
c. Completed Phase 1 and Phase 2 of the project. Program obtains program information from the input files and sends over to the Admission Office. The obtained data is finally transferred to an output file.
Admission office then receives students’ data and decides whether to admit a student to a department or not based on the output file data.
d. 
Department.c does the following-
	(a) Opens socket for each of the department to send program information from the input files.
	(b) Reads input from the respective files.
	(c) Sends each row as a packets to the Admission Office file.
	(d) Print the appropriate messages during the process.
	(e) Closes the socket.
	(f) It then waits to receive results data containing the admitted students to each department.
	(g) Prints appropriate messages to show the results. 
Admission.c does the following-
	(a) Opens socket to listen for connections.
	(b) Accepts connection for the first Department.
	(c) Reads the obtained data and updates the output file.
	(d) Once done, waits for second department connection.
	(e) Repeats the same process as done for first department.
	(f) Prints appropriate messages at various points during the process.
	(g) It then accepts student information received from Student.c
	(h) Decides the students’ admission based on the received Student data and the program information in the output file.
	(i) Sends a packet to each student containing the results.
	(j) Sends a packet containing admitted students to each department.

Phase1:
Each packet includes the following information:
A#A1#3.6 => <Department name>#<Program name>#<Required GPA>

Phase2:
Packet information with student results is as sent as follows:
Accept#A1#departmentA => <Decision>#<Program name>#<Department name>

Packet information containing admitted students to each department:
Student2#3.6#A1 => <Student>#<Required GPA>#<Program name>

Obtained output files:
Database.txt - phase 1 output file.
Include information from both the departments in the following format -
A#A1#3.6 => <Department name>#<Program name>#<Required GPA>

Compilation command(included Makefile) -

make

e. Command line format for execution- 

Code execution should be in the following order:

Open two tabs for execution.
In first tab, give the following command
./Admission

In second tab, give below command
./Department

In second tab, give below command
./Student

Run admission command before running the Department to execute successfully.

NOTE ===> Admission.c code waits only for a period of 15 sec to complete giving Student.c.

g. Failing conditions:-
Programs fails incase of invalid socket connection, binding, connect, accept and send/read commands.
Also, will not give desired results if above order is not followed.

h. Reused code:
Used partial code from Beej guide.