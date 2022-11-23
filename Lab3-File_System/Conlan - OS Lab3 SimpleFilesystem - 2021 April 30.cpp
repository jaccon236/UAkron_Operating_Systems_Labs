/*
    COPYRIGHT (C) 2021 Jacen Conlan (jcc126) All rights reserved.

    Operating Systems Concepts - 3460:426
    Lab 3 - A Simple File System
    Filename: filesystem.cpp
    Author: Jacen Conlan (jcc126@zips.uakron.edu)
    Version: 1.01 04.27.2021

    To compile this program run the command "g++ -o filesys filesystem.cpp"
    in the same directory which contains this program

    //Based on a program by Michael Black, 2007
    //Revised 11.3.2020 O'Neil
    //Revised 04.27.2021 Conlan *Academic Integrity Pledge at end of file
*/

#define DISK_SIZE 261632
#define MAX_FILE_SIZE 12288
#define MAP_LOCATION 512*256
#define DIR_LOCATION 512*257

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

// --------------------File and Sector Struct Declaration----------------------
struct file
{
    char fileName[8];
    int nameLength;
};

struct sector
{
    int sectorNumber;
    int sectorLength;
    int sectorName;
};

// -------------------------Disk Class Declaration-----------------------------
class Disk
{
    // All members are public
    public:
        FILE* dName;
        char dMap[512];
        char dFiles[512];

        // Constructor
        Disk(char*);

        // Invalid Constructors (throw error if called)
        Disk() = delete;
        Disk(Disk const&) = delete;
        Disk& operator=(Disk const&) = delete;

        // Member Functions
        void openDrive(char*);
        void closeDrive();
        void seekDrive(int);
        void writeBack();
        void setMap(FILE*);
        void setDir(FILE*);
};

// --------------------------Function Declarations-----------------------------
void locateTextFile(Disk&, sector&, file&);
void printFileContents(Disk&, sector&);
void makeTextFile(Disk&, file&);
void deleteFile(Disk&, sector&);
void listFiles(Disk&);
void error(string);
void help();

// -----------------------Simple File System Program---------------------------

int main(int argc, char* argv[])
{
    // Do nothing if no command is provided
    if (argc == 1)
    {
        error("Argument Error");
    }

    // Open floppya.img and get its map and directory data
    char diskName[256] = "floppya.img";
    Disk floppy(diskName);

    // Create structs to hold file name and sector data
    file fileData = {};
    sector sectorData = {};

    // If a file name was included fill fileData
    if (argc >= 3)
    {
        for (int i = 0; i < 8; i++)
        {
            if (argv[2][i] == '\0')
            {
                break;
            }

            else
            {
                fileData.fileName[i] = argv[2][i];
                fileData.nameLength++;
            }
        }
    }

    // Call correct functions based on argument passed
    switch(argv[1][0])
    {
        case 'L':
            if (argc != 2)
            {
                error("Argument Error");
            }

            listFiles(floppy);
            break;

        case 'P':
            if (argc != 3)
            {
                error("Argument Error");
            }

            locateTextFile(floppy, sectorData, fileData);

            switch(sectorData.sectorNumber)
            {
                case -1:
                    error("ERROR: File does not exist on the disk");
                    break;

                case -2:
                    error("File exists on the disk but is not a text file.");
                    break;

                default:
                    printFileContents(floppy, sectorData);
            }
            break;

        case 'M':
            if (argc != 3)
            {
                error("Argument Error");
            }

            locateTextFile(floppy, sectorData, fileData);

            if (sectorData.sectorNumber == -1)
            {
                makeTextFile(floppy, fileData);
            }

            else
            {
                error("ERROR: File already exists");
            }
            break;

        case 'D':
            if (argc != 3)
            {
                error("Argument Error");
            }

            locateTextFile(floppy, sectorData, fileData);

            if (sectorData.sectorNumber == -1)
            {
                error("ERROR: File does not exist");
            }

            else
            {
                deleteFile(floppy, sectorData);
            }
            break;

        default:
            error("Argument Error");
    }

    floppy.closeDrive();
}

// --------------------------Function Definitions------------------------------

// Function to list the files on the drive
// "./programName L"
void listFiles(Disk& drive)
{
    int totalBytes;

    // Search through the files located on the drive
    for (int i = 0; i < 512; i += 16)
    {
        // If a file exists at i, print it's name on screen
        if (drive.dFiles[i] != 0)
        {
            // Add size of the file to totalBytes
            totalBytes += 512*drive.dFiles[i + 10];

            for (int j = 0; j < 8; j++)
            {
                if (drive.dFiles[i + j] != 0)
                {
                    cout << drive.dFiles[i + j];
                }
            }

            // Add the file's extension
            if (tolower(drive.dFiles[i + 8]) == 't')
            {
                cout << ".t  ";
            }

            else
            {
                cout << ".x  ";
            }
        }
    }

    // Print on screen the total space, used space, and free space on the disk
    cout << endl << endl << "Space Used: " << totalBytes << "B" << endl
         << DISK_SIZE - totalBytes << "B free of " << DISK_SIZE << "B"
         << endl << endl;
}

/* Function to locate a text file on the drive

If the file exists and is a text file:
    The sector number in sectorData is set to the start sector of the file

If the file exists and is not a text file:
    The sector number in sectorData is set to -2

If the file does not exist
    The sector number in sectorData is set to -1

*/
void locateTextFile(Disk& drive, sector& sectorData, file& fileData)
{
    bool fileFound = false;
    int nameLength = 0;

    // Search through the files on the drive
    for (int i = 0; i < 512; i += 16)
    {
        // If a file exists at i:
        if (drive.dFiles[i] != 0)
        {
            // Determine the length of the file name
            nameLength = 0;
            for (int j = 0; j < 8; j++)
            {
                if (drive.dFiles[i + j])
                {
                    nameLength++;
                }
            }

            // Compare the file being searched for to the current file at i
            if (nameLength == fileData.nameLength)
            {
                for (int j = 0; j < nameLength; j++)
                {
                    if (drive.dFiles[i + j] != fileData.fileName[j])
                    {
                        fileFound = false;
                        break;
                    }

                    else if (drive.dFiles[i + j] == fileData.fileName[j])
                    {
                        fileFound = true;
                    }
                }
            }

            // If the file is found check if it is a text file
            if (fileFound)
            {
                if (tolower(drive.dFiles[i + 8]) == 't')
                {
                    sectorData.sectorNumber = drive.dFiles[i + 9];
                }

                else
                {
                    sectorData.sectorNumber = -2;
                }

                // Set the name and length fields of sectorData accordingly
                sectorData.sectorLength = drive.dFiles[i + 10];
                sectorData.sectorName = i;
                return;
            }
        }
    }

    // If no file was found set sectorData accordingly
    sectorData.sectorNumber = -1;
    sectorData.sectorLength = 0;
    sectorData.sectorName = -1;
    return;
}

// Function to print the contents of a text file
// "./programName P fileName"
void printFileContents(Disk& drive, sector& sectorData)
{
    char fileContents[MAX_FILE_SIZE];

    // Locate the start sector of the file on the drive
    drive.seekDrive(sectorData.sectorNumber);

    // Read the contents of the file into fileContents
    for (int i = 0; i < MAX_FILE_SIZE; i++)
    {
        fileContents[i] = fgetc(drive.dName);
    }

    // Print the file to the screen
    int n = 0;
    while (fileContents[n])
    {
        cout << fileContents[n];
        n++;
    }

    cout << endl;
}

// Function to make a text file
// "./programName M fileName"
void makeTextFile(Disk& drive, file& fileData)
{
    // Set variables such that no free sector or map location exist
    int freeSector = -1;
    int freeMap = -1;

    // Locate a free sector
    for (int i = 0; i < 512; i += 16)
    {
        if (drive.dFiles[i] == 0 && freeSector == -1)
        {
            freeSector = i;
            break;
        }
    }

    // Locate a free map location
    for (int i = 0; i < 16; i++)
    {
        // If a map location has been found, stop searching
        if (freeMap != -1)
        {
            break;
        }
        for (int j = 0; j < 16; j++)
        {
            if (freeMap != -1)
            {
                break;
            }

            // If current map location is free set freeMap
            if (drive.dMap[16 * i * j] == 0)
            {
                // Mark that map location is no longer free
                drive.dMap[16* i * j] = 0xff;
                freeMap = 16 * i * j;
            }
        }
    }

    // If no free map was found, disk is full; Throw error
    if (freeMap == -1)
    {
        error("ERROR: Insufficient Disk Space. Cannot Create File.");
        exit(-1);
    }

    // Otherwise, create the new text file with a name of 8 characters max
    else
    {
        if (fileData.nameLength < 8)
        {
            for (int i = 0; i < fileData.nameLength; i++)
            {
                drive.dFiles[freeSector + i] = fileData.fileName[i];
            }

            // Pad name with zeros if less than 8 characters
            for (int i = fileData.nameLength; i < 8; i++)
            {
                drive.dFiles[freeSector + i] = 0;
            }
        }

        else
        {
            for (int i = 0; i < 8; i++)
            {
                drive.dFiles[freeSector + i] = fileData.fileName[i];
            }
        }

        drive.dFiles[freeSector + 8] = 't';
        drive.dFiles[freeSector + 9] = freeMap;
        drive.dFiles[freeSector + 10] = 1;
    }

    // Prompt user to enter the contents of the file
    char buffer[512];
    cout << "Enter the text to write to the file (Max 512 characters): ";
    fgets(buffer, sizeof(buffer), stdin);

    // Move to the free map location on the drive and add the text to the file
    drive.seekDrive(freeMap);
    for (int i = 0; i < 512; i++)
    {
        fputc(buffer[i], drive.dName);
    }

    // Write back the contents of the map and directory to the drive
    drive.writeBack();
}

// Function to delete a file
// "./programName D fileName"
void deleteFile(Disk& drive, sector& sectorData)
{
    // Set the first character of the existing file name to 0
    // Essentially a 'pseudo delete'
    drive.dFiles[sectorData.sectorName] = 0;

    // Free the map location of the file to allow overwrite
    for (int i = 0; i < sectorData.sectorLength; i++)
    {
        drive.dMap[sectorData.sectorNumber] = 0x00;
        sectorData.sectorNumber++;
    }

    cout << "File Deleted" << endl;

    // Write back the contents of the map and directory to the drive
    drive.writeBack();
}

// Help prompt displayed if there is an argument error
void help()
{
    cout << "ACCEPTED ARGUMENTS AND FORMATTING" << endl << endl;

    cout << "Please run this program via this format: ./program X fileName" << endl << endl;
    cout << '\t' << right << setw(9) << "program:" << left << " the name of this compiled program" << endl;
    cout << '\t' << right << setw(9) << "X:" << left << " the command you would like to execute" << endl;
    cout << '\t' << right << setw(9) << "fileName:" << left << " the name of the file for commands that require a file name" << endl;
    cout << '\t' << right << setw(9) << " " << left << " DO NOT INCLUDE FILE EXTENSION" << endl << endl;

    cout << "Available Commands:" << endl;
    cout << "\tL: Lists the files on the drive; Do not include a file name" << endl;
    cout << "\tP: Prints the contents of a file; Requires a file name" << endl;
    cout << "\tM: Make a text file; Requires a file name; Prompts user to enter text up to 512 characters" << endl;
    cout << "\tD: Delete a file; Requires a file name; This process cannot be undone" << endl << endl;

    cout << "To see this help prompt again, run the program without a command." << endl << endl;
    return;
}

// Function to display an error and exit the program gracefully
void error(string msg)
{

    if (msg == "Argument Error")
    {
        help();
        cerr << "Invalid Arguments" << endl << "Please see above for help" << endl;
    }

    else
    {
        cerr << msg << endl;
    }

    exit(1);
}

// --------------------Disk Class Function Definitions-------------------------

// Constructor (Requires a drive name)
Disk::Disk(char* input)
{
    openDrive(input);
    setMap(dName);
    setDir(dName);
}

// Opens the drive specified
void Disk::openDrive(char* input)
{
    // Open the drive image
    dName = fopen(input,"r+");

    // Exit if drive is not found
    if (!dName)
    {
        cerr << input << " not found" << endl;
        exit(1);
    }
}

// Close the drive
void Disk::closeDrive()
{
    fclose(dName);
}

// Move to the sector specified on the drive
void Disk::seekDrive(int sector)
{
    fseek(dName, 512*sector, SEEK_SET);
}

// Write back the contents of the map and directory to the drive
void Disk::writeBack()
{
    fseek(dName, MAP_LOCATION, SEEK_SET);
    for (int i = 0; i < 512; i++)
    {
        fputc(dMap[i], dName);
    }

    fseek(dName, DIR_LOCATION, SEEK_SET);
    for (int i = 0; i < 512; i++)
    {
        fputc(dFiles[i], dName);
    }
}

// Locate the map on the drive and store the contents in dMap
void Disk::setMap(FILE* drive)
{
    fseek(drive, MAP_LOCATION, SEEK_SET);

    for(int i = 0; i < 512; i++)
    {
        dMap[i]=fgetc(drive);
    }
}

// Locate the directory of the drive and store the contents in dFiles
void Disk::setDir(FILE* drive)
{
    fseek(drive, DIR_LOCATION, SEEK_SET);
    for (int i = 0; i < 512; i++)
    {
        dFiles[i] = fgetc(drive);
    }
}

/*
// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed: Jacen Conlan       Date: 2021 April 27
*/

/*
END OF FILE
*/


