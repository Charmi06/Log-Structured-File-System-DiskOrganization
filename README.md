# Log Structured File System: Disk Organization
Implementation of a log-structured file system imitation with a focus on disk organization where the user will be able to get the text file from a Linux or Mac system and will be able to read, update, and delete the file from the log-structured file system. Here buffer cache will be handled by the host operating system memory management.

Implementation Instruction: 
implementation can be performed on both macOS and LINUX, I would recommend before executing the commands, please use the below steps.

1. make clean – This will clean the make file which was stored before in any system.
   ```bash
   make clean
2. make drive – This will create a new drive that will have segments, file mapping, and checkpoints
   ```bash
   make drive
3. make – This will run the main file and generate an executable file
   ```bash
   make
4. ./main - Run the executable
   ```bash
   ./main

# Commands to Perform
| Commands      | Description      |
|---------------|---------------|
| import | This will Import the file from the macOs or LINUX system.  |
| deletefile  | This will delete the file from the log-structured file system.  |
| containoffile  | This will list the content of the file.  |
| display | This will display the user-specific bytes of the file. |
| change  | This will change the character inside the file.  |
| clean  | This will clean the segment.  |
| exit | Will write the cached memory on the disk and exit the command prompt/Terminal.  |
| ls  | Will list the name of the file and size with active segments.  |
| currentsegment  | Display the currently active segment. |
| segmentsummary | Show the active segment summary.  |
| import items  | This will import the file n number of times.  |
| imap  | Display the imap of the active segment.  |
| checkpoint  | Show the content of the checkpoint.  |
| cleansegments | Will show the clean segments.  |
| nextblocks  | Show the next available block.  |
| clear  | Clear the screen.  |
