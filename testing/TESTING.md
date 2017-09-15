# Testing AMStartup and AmazingClient

## Argument Testing

* Tested if AMStartup doesn't crash by using different conditions that the user may input
such as negative numbers, letters or special characters for the nAvatars or Difficulty. THe 
program has intensive error checking that blocked all these invalid inputs including if the hostname is not flume.cs.dartmouth.edu.
* We used the moth.sh and the .moth file included to test to make sure that our AMStartup could properly validate the command line arguments that it was given. 

## Solving

* For each difficulty level, we tested the number of Avatars from 2 to 9 avatars waiting for the MAZE DONE message to know if the test was successful.
* After many runs, the program managed to meet the minimum requirements of solving the maze on difficulty 6 with more than 4 Avatars without receiving a too many moves message or one of the
error messages.
* The hardest combination we could consistently solve was **difficulty 8; avatars 4** after which our success rate was >50% to around difficulty 9. In difficulty 9 however, our algorithm became very dependent on the arrangment of the maze and we could not *consistently* solve the maze. 

## Valgrind
* When running `valgrind --leak-checks=full --show-leak-kinds=all` on AMStartup, it showed that all bytes were freed and thus there was no memory still reachable. 
* Running valgrind on AmazingClient seemed too challenging because AmazingClient required an open MazePort and connection with a server, so running it separately from AMStartup (ie ./AmazingClient) was too difficult.
