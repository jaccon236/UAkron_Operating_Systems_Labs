#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <limits>  // numeric_limits<std::streamsize> for validations
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LEN     80
#define MAX_ARG_LEN         16
#define MAX_ARGS            64
#define WHITESPACE          " ,\t"

using namespace std;

// Struct for user entered command
// Expects a name, number of arguments, and array
// of arguments to be initialized before use
struct command_t
{
    const char* name;
    int argc;
    char* argv[MAX_ARGS];
};

// Function Declarations
void promptUser();
bool readCommand(char*);
void parseCommand(char*, command_t*);
void executeCommand(command_t*);
void internalCommand(command_t*);
void displayHelp();

// Main Program
int main()
{
    // Necessary Variables
    char cmdLine[MAX_COMMAND_LEN];  // C-string to accept user input
    bool userInput = false;         // Detect if a command was entered or not

    // Infinite Loop
    while (true)
    {
        // Dynamically allocate space for new command_t struct
        command_t* command;
        command = new command_t;

        // Prompt user and wait for input
        promptUser();
        userInput = readCommand(cmdLine);

        // If user inputs a command: parse and execute it
        if(userInput)
        {
            parseCommand(cmdLine, command);
            internalCommand(command);
            cin.clear();
        }

        // Delete the memory allocation for command_t struct and reset userInput
        delete command;
        userInput = false;
    }
}

// Accepts no parameters
// Returns nothing
// Prompts the user to input a command
void promptUser()
{
    string promptString = "Linux(jcc126)|> ";
    cout << promptString;
}

// Accepts a pointer to a C-string
// Returns a boolean value
// Reads input from the user and stores it in the C-string which the parameter
// cmdL points to. Input includes all whitespace up to the newline character.
// Then the buffer is cleared to avoid runtime errors. If the user does not
// enter any commands, the function returns false. If a command is entered,
// it is stored in cmdL and returns true.
bool readCommand(char* cmdL)
{
    cin.get(cmdL, MAX_COMMAND_LEN, '\n');
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (!*cmdL)
    {
        // cout << "No User Input" << endl;
        return false;
    }
    return true;
}

// Accepts a pointer to a C-string and a struct of type command_t
// Returns nothing
// Parses the command in cmdL and stores the first word of the command in
// the name member of the command_t struct. Then, the entire command is
// stored word by word in the array contained in the argv member. Lastly,
// the number of arguments in the array is stored in the argc member.
void parseCommand(char* cmdL, command_t* cmd)
{
    cmd->argc = 0;
    cmd->argv[cmd->argc++] = strtok(cmdL, WHITESPACE);
    while ((cmd->argv[cmd->argc++] = strtok(NULL, WHITESPACE)) != NULL)
    {
    }

    cmd->argv[--cmd->argc] = NULL;
    cmd->name = cmd->argv[0];
}

// Accepts a pointer to a struct of type command_t
// Returns nothing
// Determines if command entered is an internal command recognized specifically
// by the scute shell as a shortcut for a known Linux command. If the command
// is a part of the shell, it will replace the name member with the correct
// Linux command and call the function executeCommand.
void internalCommand(command_t* cmd)
{
    if (strcmp(cmd->name, "Q") == 0)
    {
        exit(0);
    }

    else if (strcmp(cmd->name, "H") == 0)
    {
        displayHelp();
        return;
    }

    else if (strcmp(cmd->name, "W") == 0)
    {
        // cout << "Replacing command with clear" << endl;
        cmd->name = "clear";

    }

    else if (strcmp(cmd->name, "E") == 0)
    {
        // cout << "Replacing command with echo" << endl;
        cmd->name = "echo";

    }

    else if (strcmp(cmd->name, "L") == 0)
    {
        // cout << "Replacing command with ls" << endl;
        cmd->name = "ls";

        // Dynamically allocate space for new command_t struct
        command_t* pwd;
        pwd = new command_t;

        pwd->name = "pwd";
        pwd->argc = 1;
        pwd->argv[0] = "pwd";
        pwd->argv[1] = NULL;

        executeCommand(pwd);

        cout << endl;

        int i = 0;
        while (cmd->argv[i] != NULL)
        {
            i++;
        }

        cmd->argv[i] = "-l";
    }

    else if (strcmp(cmd->name, "P") == 0)
    {
        // cout << "Replacing command with more" << endl;
        cmd->name = "more";

    }

    else if (strcmp(cmd->name, "M") == 0)
    {
        // cout << "Replacing command with nano" << endl;
        cmd->name = "nano";

    }

    else if (strcmp(cmd->name, "C") == 0)
    {
        // cout << "Replacing command with cp" << endl;
        cmd->name = "cp";

    }

    else if (strcmp(cmd->name, "D") == 0)
    {
        // cout << "Replacing command with rm" << endl;
        cmd->name = "rm";

    }

    else if (strcmp(cmd->name, "S") == 0)
    {
        // cout << "Replacing command with cp" << endl;
        cmd->name = "firefox";

    }

    executeCommand(cmd);
}

// Accepts a pointer to a struct of type command_t
// Returns nothing
// Sets the PID to 0 and then forks the program. The child created by the fork
// executes the command by passing the name of the command and the list of
// arguments to the execvp() function. If the command passed to the function
// is not a recognized or valid command, the child will terminate and return
// control to the parent, returning to the function which called it. If the
// command is a valid command, the child will execute the command and the
// parent will wait until the child has completed its task. Once the child
// terminates, the parent resumes and returns to the function that called it.
void executeCommand(command_t* cmd)
{
    int pid = 0;

    if ((pid = fork()) == 0)
    {
        if (execvp(cmd->name, cmd->argv) < 0);
        {
            cout << "Command Not Recognized" << endl;
            exit(1);
            return;
        }
    }
    else
    {
        wait(NULL);
    }
}

// Accepts a pointer to a struct of type command_t
// Returns nothing
// Prints the help screen
void displayHelp()
{
    cout << setw(68) << "__________________________________________________________________ " << endl;
    cout << setw(68) << "|------------------------------------------------------------------|" << endl;
    cout << setw(68) << "|                    Scute Shell - Help Menu                       |" << endl;
    cout << setw(68) << "|                        List of Commands                          |" << endl;
    cout << setw(68) << "|__________________________________________________________________|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "Q: Quits the terminal"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "H: Shows the help menu"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "W: Clears the screen"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "E string: Echos the string on screen"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "L: Shows current directory and contents in long form"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "P file: Prints the contents of file on screen"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "M file: Makes or opens file in nano"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "C file1 file2: Copies the content in file1 to file2"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "D file: Deletes file"
         << right << setw(2) << "|" << endl;
    cout << left << setw(2) << "|" << setw(4) << " " << setw(60) << "S: Opens Firefox in the background"
         << right << setw(2) << "|" << endl;
    cout << setw(68) << "|__________________________________________________________________|" << endl;
    cout << endl;

}

/*
    END OF FILE
*/

