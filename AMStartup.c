/* AMStartup.c - Program responsible for connecting with the Maze server. Handles
* the communications between the avatars and the server after it
* creates N avatars. It creates the log file for all the Avatars to write to.
*
* inputs -> Number of avatars, Difficulty, Hostname
* ouputs -> Log file, N processes of the Avatar Program
*
* Special considerations: Max size of input = 1000; Assume mazewidth and height won't reach 6 figures
* Sample command line prompt: ./AMStartup 1 1 flume.cs.dartmouth.edu
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
#include <unistd.h>
#include <time.h>
#include "amazing.h"
#include "common.h"
bool check_if_integer(char * input);
int main(const int argc, char *argv[]) {

  // First check if user has /log directory in which to write log files to
  if (!IsDirectory(LOGDIRECTORY)) {
		// User does not have a /log directory in which to put log files
		fprintf(stderr, "Exiting program. Must have a log/ directory.\n");
		exit (1);
	}

  int nAvatars = 0;
  int difficulty = 0;
  char *hostname;	      // server hostname, flume.cs.dartmouth.edu
  FILE *logfile;
  struct sockaddr_in server;  // address of the server
  uint16_t port = (uint16_t)atoi(AM_SERVER_PORT);
  // check arguments
  if (argc != 4) {
    fprintf(stderr, "usage: ./AMStartup nAvatars difficulty hostname\n");
    exit(1);
  }
  else {
    // check num avatars
    if (!check_if_integer(argv[1])) {
      fprintf(stderr, "Number of avatars %s is not a whole number.\n", argv[1]);
      exit(1);
    }
    nAvatars = atoi(argv[1]);
    if (nAvatars < 2 || nAvatars > AM_MAX_AVATAR) {
      fprintf(stderr, "Number of avatars must be between 2 and 10 inclusive.\n");
      exit(1);
    }
    // check difficulty
    if (!check_if_integer(argv[2])) {
      fprintf(stderr, "Difficulty level %s is not a whole number.\n", argv[2]);
      exit(1);
    }
    difficulty = atoi(argv[2]);
    if (difficulty < 0 || difficulty > AM_MAX_DIFFICULTY) {
      fprintf(stderr, "Difficulty must be integer between 1 and 9.\n");
      exit(1);
    }
    // set hostname
    hostname = (char *)argv[3]; // flume.cs.dartmouth.edu
  }
  // Look up the hostname specified on command line
  struct hostent *hostp = gethostbyname(hostname);
  if (hostp == NULL) {
    fprintf(stderr, "Unknown host '%s'\n", hostname);
    exit(3);
  }
  // Initialize fields of the server address
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  bcopy(hostp->h_addr_list[0], &server.sin_addr, hostp->h_length);
  server.sin_port = htons(port);
  // Create socket
  int comm_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (comm_sock < 0) {
    perror("Opening socket");
    exit(2);
  }
  // And connect that socket to that server
  if (connect(comm_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
    perror("Connecting to stream socket");
    exit(4);
  }
  printf("Connected to server...\n");
  // create init message
  AM_Message message;
  AM_Message init_ok_received;
  message.type = htonl(AM_INIT);  // init
  message.init.nAvatars = htonl(nAvatars);
  message.init.Difficulty = htonl(difficulty);
  if (send(comm_sock, (AM_Message*) &message, sizeof(AM_Message), 0) <= 0) {
    perror("Sent no INIT message to server");
    exit(4);
  }
  if (recv(comm_sock, (AM_Message * ) &init_ok_received, sizeof(AM_Message), 0) <= 0) {
    //error: server terminated prematurely
   perror("Did not receive INIT_OK from server");
   close(comm_sock);
   exit(4);
  }
  init_ok_received.type = ntohl(init_ok_received.type); // AM_INIT_OK;
  // Error checking
  if (IS_AM_ERROR(init_ok_received.type)) { // some type of error at all
    if (IS_AM_INIT_ERROR(init_ok_received.type)) { //
    // AM_INIT failure
      if (init_ok_received.type == AM_INIT_ERROR_MASK) {
        if (init_ok_received.type == AM_INIT_TOO_MANY_AVATARS) fprintf(stderr, "AM_INIT_TOO_MANY_AVATARS\n");
        else if (init_ok_received.type == AM_INIT_BAD_DIFFICULTY) fprintf(stderr, "AM_INIT_BAD_DIFFICULTY\n");
      }
    }
    // not an AM_INIT_ERROR
    else if (init_ok_received.type == AM_INIT_FAILED) fprintf(stderr,"Could not initialize. INIT_OK not received. Error number:%d\n", ntohl(init_ok_received.init_failed.ErrNum) );
    else if (init_ok_received.type == AM_SERVER_DISK_QUOTA) fprintf(stderr, "AM_SERVER_DISK_QUOTA\n");
    else if (init_ok_received.type == AM_SERVER_OUT_OF_MEM) fprintf(stderr, "AM_SERVER_OUT_OF_MEM\n");
    else if (init_ok_received.type == AM_UNKNOWN_MSG_TYPE) fprintf(stderr, "Unkonwn message type. Error badtype: %d\n", ntohl(init_ok_received.unknown_msg_type.BadType));
    else if (init_ok_received.type == AM_SERVER_TIMEOUT) fprintf(stderr, "AM_SERVER_TIMEOUT\n");
    else if (init_ok_received.type == AM_UNEXPECTED_MSG_TYPE) fprintf(stderr, "AM_UNEXPECTED_MSG_TYPE\n");
    close(comm_sock);
    exit(4);
  }
  // initialization successful
  else if ( init_ok_received.type == AM_INIT_OK ) {
    init_ok_received.init_ok.MazePort = ntohl(init_ok_received.init_ok.MazePort);
    init_ok_received.init_ok.MazeWidth = ntohl(init_ok_received.init_ok.MazeWidth);
    init_ok_received.init_ok.MazeHeight = ntohl(init_ok_received.init_ok.MazeHeight);
  }
  // shouldn't print
  else {
    fprintf(stderr, "Server error, INIT_OK message not received\n");
    close(comm_sock);
    exit(4);
  }
  close(comm_sock);
  // create log file
  // Name should be Amazing_$USER_N_D.log
  char * username = getenv("USER");
  if (username == NULL) {
    fprintf(stderr, "Getting username failed. Exit.\n");
    exit(5);
  }
  // Create logfile name
  char num_of_avatars[MAXSIZE];
  sprintf(num_of_avatars, "%d", nAvatars);
  char difficulty_level[MAXSIZE];
  sprintf(difficulty_level, "%d", difficulty);
  char underscore[3] = "_";
  char extension[6] = ".log";
  char logfile_name[MAXSIZE] = "log/Amazing_";
  strcat(logfile_name, username);
  strcat(logfile_name, underscore);
  strcat(logfile_name, num_of_avatars);
  strcat(logfile_name, underscore);
  strcat(logfile_name, difficulty_level);
  strcat(logfile_name, extension);
  // get time and date
  time_t currenttime;
  struct tm * timedate;
  time(&currenttime);
  timedate = localtime(&currenttime);
  if ((logfile = fopen(logfile_name, "w")) != NULL) { // open for writing
    fprintf(logfile, "%s %d %s\n", username, init_ok_received.init_ok.MazePort, asctime(timedate));
    fclose(logfile);
  }
  else {
    fprintf(stderr, "Logfile could not be created at this time.\n");
    // free(username);
    exit(5);
  }
  // logfile first line should have $USER, Mazeport, date and time
  // Start avatar client program for each avatar - separate processes
  for( int n = 0 ; n < nAvatars; n++){
    pid_t pID = fork();
    if(pID < 0)     //failed to fork
    {
      perror("Fork failed. Unable to start process for an Avatar\n");
      exit(6);
    }
    else if ( pID == 0 ) // the child
    { //call the avatar program
      int execReturn;
      char ID[5];
      char AvatarNumber[5];
      char IPAddress[30];
      char MazePort[10];
      char MazeWidth[7];        //assuming the mazewidth and height will not reach one million
      char MazeHeight[7];
      //change client arguments to string for use in the execlp function
      sprintf(ID, "%d", n);
      sprintf(AvatarNumber, "%d", nAvatars);
      inet_ntop(AF_INET, &(server.sin_addr.s_addr), IPAddress, 30);
      sprintf(MazePort, "%d", init_ok_received.init_ok.MazePort);
      sprintf(MazeWidth, "%d", init_ok_received.init_ok.MazeWidth);
      sprintf(MazeHeight, "%d", init_ok_received.init_ok.MazeHeight);
      printf("Process started for Avatar #%s. Total avatars: %s Difficulty: %d IP address: %s Maze port: %s Maze width: %s Maze height %s \n", ID, AvatarNumber, difficulty, IPAddress, MazePort, MazeWidth, MazeHeight);
      fflush(stdin);
      execReturn = execlp("./AmazingClient", ID, AvatarNumber, IPAddress, MazePort, logfile_name, MazeWidth, MazeHeight, (char *)0);
      if( execReturn == -1){
        printf("Failed to execute the client program\n");
        exit(7);
      }
    }
  }
  return 0;
}
// check if valid difficulty
bool check_if_integer(char * input) {
      int length = strlen(input);
      // printf("string length of input %d\n", length);
      for (int i=0;i<length; i++) {
          if (!isdigit(input[i])) return false;
      }
      // printf ("Given input is a number\n");
      return true;
}
