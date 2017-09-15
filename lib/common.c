/* Common Functions
 *
 *
 *
 * Negoska 	March 2017
 */
#define MAXSTRLEN 10000

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include "common.h" // contains the data for the maze struct
#include "file.h" // readline()
#include "amazing.h"

/* Mallocs the space for a new maze */
maze_t *maze_new(int width, int height)
{
	maze_t *maze;

	maze = (maze_t *)calloc(1, sizeof(maze_t));
	if (MallocFail(maze)) return NULL;

	/* Initialize the width and height to the data from the maze */
	maze->width = width;
	maze->height = height;

	/* Malloc space for each square in maze */
	maze->square = (square_t **)calloc(height, sizeof(square_t *));
	if (MallocFail(maze->square)) return NULL;
	for (int i = 0; i < height; i++) {
		maze->square[i] = (square_t *)calloc(width, sizeof(square_t));
		if (MallocFail(maze->square[i])) return NULL;
	}

	/* Initiliaze all the data to false because we don't know the maze */
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			maze->square[i][j].crumb = false;
			maze->square[i][j].av = false;
		}
	}
	return maze;

}
/* Frees a maze */
void maze_delete(maze_t *maze)
{
	/* Free each square, then the square member then the whole maze */
	for (int i = 0; i < maze->height; i++) {
		free(maze->square[i]);
	}
	free(maze->square);
	free(maze);

	return;
}
/*Set a mazesquare to now that an avatar is there */
void maze_set(maze_t *maze, int avatarID, XYPos pos)
{
	/* Note that in this construction (x,y) are flipped from their usual  interpretation */
	maze->square[pos.y][pos.x].av = true;

	return;
}
/* Prints the maze, either to a file or the screen */
bool maze_print(maze_t maze, char *mazefile, char *logfile, int MazePort)
{
	char **mazeprint;

	/* character array of size = 2*maze.size + 1 bc. need room for walls AND avatars */
	mazeprint = (char **)calloc(2*maze.height + 1, sizeof(char *));
	if(MallocFail(mazeprint)) return false;

	for (int i = 0; i < 2*maze.height + 1; i++) {
		mazeprint[i] = (char *)calloc(2*maze.width + 1, sizeof(char));
		if (MallocFail(mazeprint[i])) return false;
	}

	/* Set all squares in the maze to empty */
	for (int i = 0; i < 2*maze.height+1; i++) {
		for (int j = 0; j < 2*maze.width+1; j++) {
			mazeprint[i][j] = '~';
		}
	}

	/* If an avatar is currently on that square set it to true */
	for (int i = 0; i < maze.height; i++) {
		for (int j = 0; j < maze.width; j++) {
			if (maze.square[i][j].crumb) mazeprint[2*i+1][2*j+1] = '*';
			//if (maze.square[i][j].topW) mazeprint[2*i][2*j+1] = '-';
			//if (maze.square[i][j].botW) mazeprint[2*i+2][2*j+1] = '-';
			//if (maze.square[i][j].leftW) mazeprint[2*i+1][2*j] = '|';
			//if (maze.square[i][j].rightW) mazeprint[2*i+1][2*j+2] = '|';
			if (maze.square[i][j].av) mazeprint[2*i+1][2*j+1] = '@';
		}
	}

	/* Get the info for the walls in the maze from the log file */
	char *line;
	FILE *fp = fopen(logfile, "r");

	if (FileFail(fp)) return false;

	/* The first two lines are not info about the walls */
	for (int i = 0; i < 2; i++) {
		line = readline(fp);
		free(line);
	}

	char *ch;
	for (int i = 0; i < maze.height; i++) {
		for (int j = 0; j < maze.width; j++) {
			line = readline(fp);

			/* For each line you want to get 1 character past the second ':' to read the walls */
			ch = line;
			for (int k = 0; k < 2; k++) {
				for(; *ch != ':'; ch++) {
					// do nothing
				}
				ch++;
			}

			/* The middle 4 square contains wall data (WNSE) */
			for (int k = 0; k < 6; k++) {
				if (k != 0 && k != 5) {
					if (*ch == 'W') {
						mazeprint[2*i+1][2*j] = '|';
					} else if (*ch == 'N') {
			 			mazeprint[2*i][2*j+1] = '-';
					} else if (*ch == 'S') {
			 			mazeprint[2*i+2][2*j+1] = '-';
					} else if (*ch == 'E') {
		 				mazeprint[2*i+1][2*j+2] = '|';
					}
				}
				ch++;
			}

			free(line);
		}
	}

	fclose(fp);

	if (strcmp(mazefile,"stdout") == 0) fp = stdout;
	else fp = fopen(mazefile, "w");

	if(FileFail(fp)) return false;

	/* Print every character in the array */
	fprintf(fp, "\n");
	for (int i = 0; i < 2*maze.height+1; i++) {

		for (int j = 0; j < 2*maze.width+1; j++) {
			if(mazeprint[i][j] == '~') fprintf(fp, " ");
			else fprintf(fp,"%c", mazeprint[i][j]);
		}
		fprintf(fp, "\n");


	}


	/* Should not fclose() stdout */
	if (fp != stdout) fclose(fp);

	/* Clean up */
	for (int i = 0; i < 2*maze.height+1; i++) {
		free(mazeprint[i]);
	}

	free(mazeprint);
	return true;
}
/* the counterpart to getLog, this function removes the log after its use is done (maze finished). */
bool removeLog(int MazePort)
{
	pid_t pid;
	
	pid = fork();
	
	if (pid == 0) {
		char logfile[MAXSTRLEN];
		sprintf(logfile, "%d.log", MazePort);
			
		if(execlp("rm", "rm", "-f", logfile, (char *)NULL) < 0) return false;
	} else {
		wait(NULL);
	}
	
	return true;

}

/*
bool getPNG(char destination[], int MazePort)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		char pnglocation[MAXSTRLEN];
		sprintf(pnglocation, "/var/tmp/%d/.png", MazePort);
		if(execlp("cp", "cp", pnglocation, destination, (char *)NULL) < 0) return false;

	} else {
		wait(NULL);
	}

	return true;


}
*/

/* THIS FUNCTION IS DEFUNCT
 *
 * All graphics are now performed in REALTIME, so this function is now NEVER called within
 * our AmazingClient because of its dependency on directory structure and its need to
 * create many files that hold pictures of the maze. This was bloating our directory.
 * Thus, all pics of the maze are now just printed to the screen
 */
/* this function prints all the files that have stored the maze, NOT performed if REALTIME */
bool runGraphics(char directory[], int MazePort, int numTurns)
{
	/* For the # of turns, cat the mazefile for that turn, perform a small delay */
	for (int i = 0; i < numTurns; i++) {

		pid_t pid;
		if((pid = fork()) < 0) return false;

		if (pid == 0) {
			char target[MAXSTRLEN];
			sprintf(target, "%s/%d_%d",directory, MazePort, i);

			int execReturn;
			if ((execReturn = execlp("cat", "cat", target, (char *)NULL)) < 0) return false;
		} else {
			/* The parent waits for the child to cat + 0.025 seconds */
	       		wait(NULL);

			/* sleep for 0.25 seconds (= 250000 microseconds) */
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 25000;
			select(0, NULL, NULL, NULL,&tv);
		}
	}
	return true;
}
/* Obtain the log file from the /var/tmp folder */
char *getLog(char directory[], int MazePort, char destination[], int AvatarId)
{
	char filepath[MAXSTRLEN];
	char target[MAXSTRLEN];
	char *rename;
	sprintf(filepath, "%s/%d/log.out", directory, MazePort); // obtain from /var/tmp; not relative

	if (AvatarId != 0) {
		/* if not avatar 0, then only find the pathname of the where this file is going */
		rename = (char *)calloc(MAXSTRLEN, sizeof(char));
		sprintf(rename, "%s/%d.log", destination, MazePort);
		printf("%s\n", rename);
		return rename;
	}

	/* Fork a new process to cp the file to our local directory */
	pid_t pid = fork();
	if (pid == 0) {
		//printf("running copy\n");
	execlp("cp", "cp", filepath, destination, (char *)NULL);
	} else {

		wait(NULL);

		rename = (char *)calloc(MAXSTRLEN, sizeof(char));

		sprintf(target, "%s/log.out", destination);
		sprintf(rename, "%s/%d.log", destination, MazePort);

		/* Change its name so that we don't have many copies of log.out */
		pid_t pid2 = fork();
		if (pid2 == 0) {
			//printf("running rename\n");
			execlp("mv", "mv", target, rename, (char *)NULL);
		} else {
			wait(NULL);
		}
	}

	return rename;
}

/* Check if a file failed to open */
bool FileFail(FILE *fp)
{
	if (fp == NULL) {
		fprintf(stderr, "An error occurred opening the file\n");
		return true;
	} else {
		return false;
	}
}
/* Check if a malloc (ptr == NULL) */
bool MallocFail(void *ptr)
{
	if (ptr == NULL) {
		fprintf(stderr, "An error occurred while mallocing\n");
		return true;
	} else {
		return false;
	}
}

// Does a /log directory exist
bool IsDirectory(char *dir) {
		FILE *file;
		char filename[100];
		strcpy(filename, dir);
		strcat(filename, "/.log");
		if ((file = fopen(filename, "w")) == NULL) {
				fprintf(stderr, "Directory is not a directory, not readable, or not a crawler's output directory\n");
			 //  free(filename);
				return false;
		}
		else {
				fclose(file);
			 //  free(filename);
				return true;
		}
}
