#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 80 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

static int timer_expired = 0;

static void alarm_handler(int sig) {
	timer_expired = 30;
}

int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;

 //Create a socket for the socket
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  	perror("Problem in creating the socket");
  	exit(2);
 }

 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port =  htons((int) strtol(argv[1], (char **)NULL, 10));

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 sigaction(SIGALRM, &(struct sigaction) {.sa_handler = alarm_handler}, NULL);
 time_t t;
 srand((unsigned) time(&t));
 timer_expired = 0;
 alarm(300);

 for ( ; ; ) {

  while (timer_expired) {
  	printf("Time expired!");
	exit(0);
  }

  clilen = sizeof(cliaddr);
  //accept a connection

  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
  printf("%s\n","Received request...");

  if ( (childpid = fork ()) == 0 ) {	//if it’s 0, it’s child process
    	printf ("%s\n","Child created for dealing with client requests");
    	//close listening socket
    	close (listenfd);

    	while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
     	//printf("%s","String received from and resent to the client:");
	
	if (strcmp(buf, "end\n") == 0) exit(0);

	FILE *fp; char result[MAXLINE]; char output[MAXLINE];
	fp = popen(buf, "r");
	while (fgets(result, sizeof(result) - 1, fp) != NULL)
		strcat(output, result);
	send(connfd, output, MAXLINE, 0);
	pclose(fp);

	int i;
	for (i = 0; i < MAXLINE; i++)
		buf[i] = '\0';
	for (i = 0; i < MAXLINE; i++)
		result[i] = '\0';
	for (i = 0; i < MAXLINE; i++)
		output[i] = '\0';
     }
     if (n < 0)
      	printf("%s\n", "Read error");
     exit(0);
  }
  //close socket of the server
  close(connfd);
 }
}	
