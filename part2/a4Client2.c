#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 10010 /*port*/

static int timer_expired = 0;

static void alarm_handler(int sig) {
	timer_expired = 30;
}

int
main(int argc, char **argv) 
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINE], recvline[MAXLINE];
 
 char* fileName;
 fileName = argv[3];
 FILE *file = fopen(fileName, "r");
 char* oFileName;
 oFileName = argv[4];
 FILE *oFile = fopen(oFileName, "w");

 // alarm(300);  // to terminate after 300 seconds
	
 //basic check of the arguments
 //additional checks can be inserted
 if (argc !=5) {
  perror("Usage: TCPClient <Server IP> <Server Port>"); 
  exit(1);
 }
	
 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }
	
 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr= inet_addr(argv[1]);
 servaddr.sin_port =  htons(atoi(argv[2]));
	
 //Connection of the client to the socket 
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  perror("Problem in connecting to the server");
  exit(3);
 }
	
 //while (fgets(sendline, MAXLINE, stdin) != NULL) {
 char line[128];

 sigaction(SIGALRM, &(struct sigaction) {.sa_handler = alarm_handler}, NULL);
 time_t t;
 srand((unsigned) time(&t));
 timer_expired = 0;
 alarm(300);

 while (fgets(line, sizeof(line), file) != NULL) {

    if (timer_expired) {
	printf("Time expired!");
	exit(0);
    }

  fprintf(oFile, "\n(Before) Client %d\n", getpid());
  FILE *fp; char time[100];
  fp = popen("date", "r");
  while (fgets(time, sizeof(time) - 1, fp) != NULL)
  	fprintf(oFile, "%s", time);
  close(fp);

  send(sockfd, line, strlen(line), 0);
  sleep(3);

  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   exit(4);
  }
  fprintf(oFile, "%s", "\nString received from the server:");
  fprintf(oFile, "%s", recvline);
  
  fprintf(oFile, "\n(After) Client %d", getpid());
  fp = popen("date", "r");
  while (fgets(time, sizeof(time) - 1, fp) != NULL)
  	fprintf(oFile, "%s", time);
  fprintf(oFile, "\n\n");
  close(fp);

  int i;
  for (i = 0; i < 128; i++)
  	line[i] = '\0';
  for (i = 0; i < MAXLINE; i++)
  	recvline[i] = '\0';
 }

 fclose(file);
 fclose(oFile);
 exit(0);
}
