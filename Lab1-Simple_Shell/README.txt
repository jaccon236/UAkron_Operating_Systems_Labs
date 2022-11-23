README for scuteShell v1.0

This is a README documentation for the scuteShell v1.0 written in C++ by Jacen Conlan (jcc126) as a project for the Operating Systems course at The University of Akron.

To compile the program (Linux System Only):
1) save scute.cpp to the directory you wish to run the program in
2) make sure g++ is updated to the most recent version
3) enter "g++ scute.cpp" into the terminal and press the return key
4) once the program has completed compilation, enter "./a.out" and press the return key
5) Entering the command "H" will bring up a help menu to show all commands and functions

All native Linux commands will work in this terminal as well as the following commands.

Q: Quits the terminal
H: Displays a help page
W: Clears the screen
L: Lists the current directory and it's contents in long form
P 'file': Prints the contents of file to the screen
M 'file': Creates and/or opens file with the nano text editor
C 'file1' 'file2': Copies the contents of file1 into file2
D 'file': Deletes file
S: Opens firefox if it is installed on the machine
E 'string': prints whatever is typed in place of string to the screen


**Important Note**
Due to time constraints and discovering the bug super late in the process, if the command "E" is called, some commands such as "L" will not work until a command other than "E" that accepts an argument is used before it. For testing purposes, I recommend testing "E" last.

**Important Note**
I do not own a Linux machine and have no way to test if "S" works properly if at all. It may not work.