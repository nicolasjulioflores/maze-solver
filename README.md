# README for Amazing Project

### Negoska Team Members 
* Taringana Guranungo
* Sharon Bian
* Nick Flores

## **IMPORTANT ASSUMPTIONS**

### If the program is not running please **comment out** #define GRAPHICS

* Server
This program **must** be run on `flume.cs.dartmouth.edu` for the graphics to work (you must be logged in here). This is because the graphics use the log file produced by the server to produce the walls for the maze. 

* `#define GRAPHICS` 
The second assumption for viewing the graphics is that `#define GRAPHICS` is written at the top of the code. If this is not written then only the textual output will be displayed. **However** the statement included above about commenting out `#define GRAPHICS` is very important as the graphics functions *may* throw an error that has to do with an incorrect server (not on flume) or incorrect directory structure (this should be fixed). Our maze works perfectly fine without the graphics. 

* Must have a log folder (see first bullet in Compiling and Running). 

## Program Description
* AMStartup intializes the communication with the server and get a port number in return that it sends to AmazingClient.c programs which are run from within it. Together with the port number, AMStartup also gets the Maze width and Maze height from the server and pass them to the AmazingClient.c process that it starts.

* The AmazingCLient are starting using process through the means of forking and calling the execlp() function in all the cild processes to replace the child AMStartup program with an iinstant of AmazingClient. This is done within a loop to produce the needed number of client instances as specified by the user. 

* The Clients solve the maze using an algorithm of left wall following until they all reach the center. If an avatar reaches the center before others, it will wait at the center of the maze until
all other avatars reach the center.

## Compiling and Running AMSstartup
* User must have a log/ directory. AMStartup and AmazingClient will prompt the user to create a log/ directory if a log/ directory is not valid, writeable, or just doesn't exist. log/ will hold all maze .log files

* To solve the maze, one uses AMStartup.c by using the makefile. Typing make will compile both the AMStartup.c and the AmazingClient.c program in the current Amazing directory. 

* To execute the program one needs run only AMStartup.c by entering  **4** arguments in this order:
./AMStartup nAvatars Difficulty Hostname; with the hostname being flume.cs.dartmouth.edu

* The arguments need to be exactly 4.

* The number of Avatars (nAvatars) should be a positive number greater than 1, and the Difficulty should be between 0 to 9 (inclusive)

* To display graphics, one needs to uncomment the #DEFINE GRAPHICS and comment out #DEFINE REALTIME. You also need to include a folder called logs and within logs, you need to include two folders called maze and logserver. 

* If the maze is solved and graphics is turned on, the avatars will all be grouped together (so it looks like there is only one avatar on the screen). However, after the maze is finished, it will appear as if there are two avatars on screen. This does not mean the maze has not been solved.
