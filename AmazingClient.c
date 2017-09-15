/* Avatar.c - moves the avatars in the maze turn by turn when it is
 * as a client instant until the maze is solved(they all reach each other). 
 * There are nAvatars  number of these spawned by AMStartup as different 
 * processes. Makes use of AMStartup.c in comunicating with the server.
 *
 * inputs -> avatarId, nAvatars, ip, mazeport, logfile, mazewidth, mazeheight
 * outputs -> (ASCII maze OR text updates) AND edited logfile (moves and success/error)
 *
 * Special considerations: MAXSTRLEN = 10000, so any logfiles can't be absurdly long in length
 * No sample command line prompt; this function is called by AMStartup.
 *
 * Negoska 				March 2017
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include "amazing.h"
#include "common.h"
#define MAXSTRLEN 10000

/* Enables ASCII graphics */
#define GRAPHICS
#define DELAY 0

/* function prototype*/
bool writeLogSuccess(char *filename, AM_Message message);
bool writeLogMove(char *filename, int avatarID, XYPos pos, int direction_of_move);
bool writeLogError(char *filename, int error);

/* Process for this avatar */
int main( int argc, char *argv[]){
	// checking number of args is sufficient
	if (argc != 7) {
		fprintf(stderr, "Incorrect argument count for avatar %d.  Usage ./AmazingClient [id] [nAvatars] [ip] [mazePort] [fileName] [mazewidth] [mazeheight]\n", atoi(argv[1]));
			exit (1);
	}
	struct sockaddr_in servaddr;  // address of the server

	/* Store arguments */
	int AvatarId = atoi(argv[0]);
	int nAvatars = atoi(argv[1]);
	// uint32_t IPaddress is argv[2]
	uint32_t MazePort = (uint32_t)atoi(argv[3]);
	char * filename = argv[4];
	int mazeWidth = atoi(argv[5]);
	int mazeHeight = atoi(argv[6]);

	if (!IsDirectory(LOGDIRECTORY)) {
		// User does not have a /log directory in which to put log files
		if (AvatarId == 0) fprintf(stderr, "Exiting maze program. Must have a log/ directory.\n");
		exit (1);
	}

	/*compass Array stores directions in a clockwise format */
	uint32_t compass[M_NUM_DIRECTIONS] = {M_WEST, M_NORTH, M_EAST, M_SOUTH};


	/* Initialize fields of the server address */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(argv[2]); // IP address
	servaddr.sin_port = htons(MazePort);

	/* Create socket */
	int comm_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (comm_sock < 0) {
    perror("opening socket");
    exit(2);
  }
  printf("Socket opened, number %d\n", comm_sock);
  /* connect that socket to that server */
  if (connect(comm_sock, (struct sockaddr *) &servaddr, sizeof(AM_Message)) < 0) {
    perror("connecting to stream socket");
    exit(4);
  }
  printf("Connected!\n");

	/* Send the ready message */
	AM_Message ReadyMessage;
	ReadyMessage.type = htonl(AM_AVATAR_READY);
	ReadyMessage.avatar_ready.AvatarId = htonl(AvatarId);
	if (send(comm_sock, (AM_Message * ) &ReadyMessage, sizeof(AM_Message), 0) <= 0) {
    perror("Sent no ReadyMessage to server");
    exit(4);
  	}
 	AM_Message ServerMessage; // FROM server
	AM_Message RequestMove; // TO server
	/* RecentPos tracks the last four positions of the avatar */
	XYPos RecentPos[4];
	XYPos initial_pos; // initialize position
	initial_pos.x = -1;
	initial_pos.y = -1;

	for (int i = 0; i < 4; i ++) RecentPos[i] = initial_pos;
	int i = 0; // index into RecentPos; counter of moves
	int d = 0; // index into compass (first look west)
	int flag = 1; // 1 means unblocked; 0 means blocked; 2 means change direction
	/* Grabs the log file from the server and uses that to display graphics */
	#ifdef GRAPHICS
	char *mazelog;
	mazelog = getLog("/var/tmp",  (int)MazePort, ".", AvatarId);
	#endif
	/* This while loop waits to receives a message from the server and acts accordingly based on
 	 * the information. If(error) -> print error. If(turn) -> send move. If solved ->
 	 * end program.
 	 */
	while(recv(comm_sock, (AM_Message * ) &ServerMessage, sizeof(AM_Message), 0) > 0) {
		ServerMessage.type = ntohl(ServerMessage.type);

		/* if(error) */
		if (IS_AM_ERROR(ServerMessage.type)) {
			if (ServerMessage.type == AM_NO_SUCH_AVATAR) fprintf(stderr, "Avatar ID %d is invalid. Exit.\n", AvatarId);
			else if (ServerMessage.type == AM_AVATAR_OUT_OF_TURN) fprintf(stderr, "AM_AVATAR_OUT_OF_TURN\n");
			else if (ServerMessage.type == AM_NO_SUCH_AVATAR) fprintf(stderr, "AM_NO_SUCH_AVATAR\n");
			else if (ServerMessage.type == AM_TOO_MANY_MOVES) fprintf(stderr, "AM_TOO_MANY_MOVES\n");
			else if (ServerMessage.type == AM_INIT_FAILED) fprintf(stderr,"Could not initialize. INIT_OK not received. Error number:%d\n", ntohl(ServerMessage.init_failed.ErrNum) );
  		else if (ServerMessage.type == AM_SERVER_DISK_QUOTA) fprintf(stderr, "AM_SERVER_DISK_QUOTA\n");
  		else if (ServerMessage.type == AM_SERVER_OUT_OF_MEM) fprintf(stderr, "AM_SERVER_OUT_OF_MEM\n");
  		else if (ServerMessage.type == AM_UNKNOWN_MSG_TYPE) fprintf(stderr, "Unknown message type. Error badtype: %d\n", ntohl(ServerMessage.unknown_msg_type.BadType));
  		else if (ServerMessage.type == AM_SERVER_TIMEOUT) fprintf(stderr, "AM_SERVER_TIMEOUT\n");
  		else if (ServerMessage.type == AM_UNEXPECTED_MSG_TYPE) fprintf(stderr, "AM_UNEXPECTED_MSG_TYPE: %d\n", ntohl(ServerMessage.type));
			else if (ServerMessage.type == AM_AVATAR_OUT_OF_TURN) fprintf(stderr, "AM_AVATAR_OUT_OF_TURN\n");
			else fprintf(stderr, "Error wasn't caught. message type = %d\n", ServerMessage.type);
			fprintf(stderr, "Error. Avatar %d not ready.\n", AvatarId);
			if (AvatarId == 0) {
				writeLogError(filename, ServerMessage.type);
				close(comm_sock);

				#ifdef GRAPHICS
				removeLog((int) MazePort);
				#endif

			}
			exit(4);
		}

		/* if(turn) */
		else if ( ServerMessage.type == AM_AVATAR_TURN ) {
			ServerMessage.avatar_turn.TurnId = ntohl(ServerMessage.avatar_turn.TurnId); // whose turn is it?

			/* iterate over SeverMessage.avatar_turn.Pos and ntohl() all the members */
			for (int pos = 0; pos < nAvatars; pos++) {
				ServerMessage.avatar_turn.Pos[pos].x = ntohl(ServerMessage.avatar_turn.Pos[pos].x);
				ServerMessage.avatar_turn.Pos[pos].y = ntohl(ServerMessage.avatar_turn.Pos[pos].y);
			}

			/* if its our turn, then move */
			if( AvatarId == ServerMessage.avatar_turn.TurnId ) {
				/* Only the Avatars whose turn it is gets to draw the maze */
				#ifdef GRAPHICS

				maze_t *maze;
				maze = maze_new(mazeWidth, mazeHeight);
				for (int pos = 0; pos < nAvatars; pos++) {
					maze_set(maze, pos, ServerMessage.avatar_turn.Pos[pos]);
				}

				/* sleep for 0.025 seconds (= 25000 microseconds) */
				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = (DELAY);
				select(0, NULL, NULL, NULL,&tv);
				maze_print(*maze, "stdout", mazelog, (int)MazePort);

				maze_delete(maze);
				#endif

				RequestMove.type = htonl(AM_AVATAR_MOVE);
				RequestMove.avatar_move.AvatarId = htonl(AvatarId);
				/* if the avatar didn't change position, it has hit a wall and must turn clockwise */
				if( RecentPos[(i - 1) % 4].x == RecentPos[( i - 2) % 4].x && RecentPos[(i - 1) % 4].y == RecentPos[( i - 2) % 4].y) {
					d = ( d + 1 ) % 4 ; //change the direction the avatar is moving in
					flag = 0;
				} else flag = 2;
		/* stop moving avatar when it reaches the center */
	      if( ((ServerMessage.avatar_turn.Pos[AvatarId].x) == mazeWidth/2) && (ServerMessage.avatar_turn.Pos[AvatarId].y == mazeHeight/2) ) {

				RequestMove.avatar_move.Direction = htonl(M_NULL_MOVE);
				#ifndef GRAPHICS
				printf("Avatar %d has reached the center. will stay still\n", AvatarId);
				#endif
				}
				/* If still searching for the center */
				else {
					if( flag == 1) { // unblocked (the default before we hit a wall and flag start changing between 0 and 2)
						RequestMove.avatar_move.Direction = htonl((uint32_t)compass[d]);
					}
					else if(flag == 2) {
						d = ( d - 1 ) % 4;
						if( d < 0 ) d = d + 4; //math lib only supports (%) for # > 0, so add 4 if d < 0.

						RequestMove.avatar_move.Direction = htonl((uint32_t)compass[d]);
						flag = 0; //set the flag to 0 since we now know what our new left direction
					}
					else if(flag == 0) { //move in the new incremented direction since we hit a wall in the previous old direction
						RequestMove.avatar_move.Direction = htonl((uint32_t)compass[d]);
					}
					if (!writeLogMove(filename, AvatarId, ServerMessage.avatar_turn.Pos[AvatarId], compass[d])) exit(6);

				}

				/* send the move message */
				if (send(comm_sock, (AM_Message * ) &RequestMove, sizeof(AM_Message), 0) <= 0) {
	  				perror("Sent no RequestMove to server");
	  				exit(4);
				}
				#ifndef GRAPHICS
				printf("Avatar #%d @ (x,y) = (%d, %d) requests move in direction %d \n", AvatarId, ServerMessage.avatar_turn.Pos[AvatarId].x, ServerMessage.avatar_turn.Pos[AvatarId].y , compass[d]);
				#endif
			}
			/* Updating RecentPos array: update my position if it was my move the turn before */
			/* If its not avatar 0 and I was the previous avatar */
			else if( (ServerMessage.avatar_turn.TurnId != 0) && (AvatarId == ServerMessage.avatar_turn.TurnId - 1) ) {

				RecentPos[i % 4] = ServerMessage.avatar_turn.Pos[AvatarId]; //put the new Avatar's position in RecentPos array
				i++; //move the index to the next slot
			}
			/* if it is avatar 0 and my ID is the last in the list */
			else if( ServerMessage.avatar_turn.TurnId == 0 && AvatarId == nAvatars - 1 ){

				RecentPos[i % 4] = ServerMessage.avatar_turn.Pos[AvatarId];
				i++;
	    		}
		 }
	/* If(maze_solved) clean up; write to log; may be run graphics; return 0 */
	if ( ServerMessage.type == AM_MAZE_SOLVED ) {

		/* Clean up */
		#ifdef GRAPHICS
		free(mazelog);
		removeLog((int)MazePort);
		#endif

		/* 0th avatar writes to the log and it runs graphics */
		if( AvatarId == 0 ){
			if(!writeLogSuccess(filename, ServerMessage)) exit (6);
			printf("MAZE DONE\n");
			close(comm_sock);

			/* CURRENTLY BROKEN
			#ifdef GETPNG
			if(!getPNG("logs/pics", (int)MazePort)) return -1;
			else printf("Stored pngs in logs/pics/\n");
			#endif
			*/

			return 0;
		}
		else { //other avatars do nothing
			printf("Maze finished\n");
			// close(comm_sock);
			return 0;
		}
	}
	}
	return 0;
}
/* Func family: writeLog()
 *
 * Description: These functions are the ones used to write to the logfile. There are three things we write to the logfile:
 * 1) move 2) success 3) errors. Each of these accepts different parameters but all are generally staightfoward. They all use
 * the FileFail(fp) function written in common.c to ensure that file opening occurred successfully.
 */
bool writeLogMove(char *filename, int avatarID, XYPos pos, int direction_of_move) {
	FILE *fp;
	fp = fopen(filename, "a");
	if (FileFail(fp)) {
		fprintf(stderr, "Error: writing to log file (move) failed.\n");
		return false;
	}
	fprintf(fp, "Avatar id: %d; (x,y) = (%d, %d); move = %d \n", avatarID, pos.x, pos.y, direction_of_move);
	fclose(fp);
	return true;
}
bool writeLogSuccess(char *filename, AM_Message message)
{
	FILE *fp;
	fp = fopen(filename, "a");
	if (FileFail(fp)) {
		fprintf(stderr, "Error: writing to log file (success) failed.\n");
		return false;
	}
	if (message.type != AM_MAZE_SOLVED) return false;
	fprintf(fp, "MAZE SOLVED; number avatars = %d; difficulty = %d; number moves = %d; hash = %d \n", ntohl(message.maze_solved.nAvatars), ntohl(message.maze_solved.Difficulty), ntohl(message.maze_solved.nMoves), ntohl(message.maze_solved.Hash));
	fclose(fp);
	return true;
}
bool writeLogError(char *filename, int error) {
	FILE *fp;
	fp = fopen(filename, "a");
	if (FileFail(fp)) {
		fprintf(stderr, "Error: writing to log file (error) failed.\n");
		return false;
	}
	fprintf(fp, "MAZE ERROR; message type: %d \n", error);
	fclose(fp);
	return true;
}
