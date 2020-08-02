a. Name: Sanjay Sana
b. USC ID: 85818009897
c. Completed Phase 1 of the project. Program obtains program information from the input files and sends over to the Admission Office. The obtained data is finally transferred to an output file.
d. 
Department.c does the following-
	(a) Opens socket for each of the department to send program information from the input files.
	(b) Reads input from the respective files.
	(c) Sends each row as a packets to the Admission Office file.
	(d) Print the appropriate messages during the process.
	(e) Closes the socket.
Admission.c does the following-
	(a) Opens socket to listen for connections.
	(b) Accepts connection for the first Department.
	(c) Reads the obtained data and updates the output file.
	(d) Once done, waits for second department connection.
	(e) Repeats the same process as done for first department.
	(f) Prints appropriate messages at various points during the process.

Each packet includes the following information:
A#A1#3.6 => <Department name>#<Program name>#<Required GPA>

Obtained output files:
Database.txt - phase 1 output file.
Include information from both the departments in the following format -
A#A1#3.6 => <Department name>#<Program name>#<Required GPA>

Compilation command(included Makefile) -

make

e. Command line format for execution- 

Open two tabs for execution.
In first tab, give the following command
./Admission

In second tab, give below command
./Department

Run admission command before running the Department to execute successfully.

g. Failing conditions:-
Programs fails incase of invalid socket connection, binding, connect, accept and send/read commands.

h. Reused code:
Used partial code from Beej guide.