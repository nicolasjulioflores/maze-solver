/* Header file for Common Functions
 *
 *
 *
 *
 * Negoska	March 2017
 */

#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "amazing.h"

typedef struct square_t {
	bool crumb;
	bool av;
} square_t;

typedef struct maze_t {
	int height;
	int width;
	square_t **square;

} maze_t;

/* maze funcs */
maze_t *maze_new(int width, int height);
void maze_delete(maze_t *maze);
void maze_set(maze_t *maze, int avatarID, XYPos pos);
bool maze_print(maze_t maze, char *mazefile, char *logfile, int MazePort);
char *getLog(char directory[], int Mazeport, char destination[], int AvatarId);
bool runGraphics(char directory[], int MazePort, int numTurns);
//bool getPNG(char destination[], int MazePort);
bool removeLog(int MazePort);
bool FileFail(FILE *fp);
bool MallocFail(void *ptr);
bool IsDirectory(char *dir);
#endif
