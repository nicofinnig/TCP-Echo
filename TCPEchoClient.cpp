/** TCPEchoClient.cpp
 * 
 * Normal build with DNS support
 * $ g++ -std=c++14 -pthread TCPEchoClient.cpp -o executables/TCPEchoClient
 * -pthread not needed for client
 * To Find container ip:
 * Run command: ./TCPEchoClient <host> <port> <message> <optional_loop_cnt>
 * Example:
 * ./TCPEchoClient ubuntu-2204_server 8080 "Hello from TCP Client" 1
 * if using loadbalancer:
 * Example:
 * ./TCPEchoClient nginx-load-balancer 8008 "Test" 2 
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 8080
#define MAXLINE (1024)
#define HOST "127.0.0.1"

void sendAndReceive(int sockfd, const char* message, char* buffer, const char* host, int port);

int main(int argc, char *argv[]) {
  int sockfd;
  char buffer[MAXLINE] = {0};
  struct sockaddr_in serveraddr;  
  char *host;
  char *message;
  int port = PORT;
  char host_ip[32];
  int n;
  int loop_cnt = 1;

  // cmd line parameters
  // - argv[1] host
  // - argv[2] host port
  // - argv[3] message
  // - argv[4] [loop_cnt] optional
  if(argc < 4) {  
    perror("Usage: TCPEchoClient <host> <port> <message> <optional_loop_cnt>\n");
    exit(EXIT_SUCCESS);
  }
    host = argv[1]; 
    port = atoi(argv[2]);
    message = argv[3]; 
    if(argc == 5) {
      loop_cnt = atoi(argv[4]); 
    }

  // create socket
  if((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0) { 
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  // reset serveraddr to 0
  memset(&serveraddr, 0, sizeof(serveraddr));  
// ehdollinen kääntäminen ifndef...
#ifdef NO_DNS_SUPPORT
  // No dns support
  serveraddr.sin_family = AF_INET;  
  serveraddr.sin_port = htons(port);
  serveraddr.sin_addr.s_addr = inet_addr(host); // Hostin osoite.
#else
  // DNS support
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(port);

  struct hostent *h;
  struct in_addr **AddrList; 

  if((h = gethostbyname(host)) == NULL) {
    printf("gethostbyname error...using original\n");
    strcpy(host_ip, host);  
  } else {
    AddrList = (struct in_addr**)h->h_addr_list;  
    strcpy(host_ip, inet_ntoa(*AddrList[0])); 
  }
  if(inet_pton(AF_INET, host_ip, &serveraddr.sin_addr) <= 0) {  
    perror("Invalid address\n");
    exit(EXIT_FAILURE);
  }
  printf("host %s -> host ip %s with loop_cnt %d\n", host, host_ip, loop_cnt);
#endif

  // connect
  if(connect(sockfd, (const sockaddr*)&serveraddr,sizeof(serveraddr)) < 0) { 
    perror("connect to server failed\n"); 
    exit(EXIT_FAILURE);
  }

  int i;

  // infinite loop if given loop_cnt -1
  if (loop_cnt == -1) {
    for(;;) { 
      sendAndReceive(sockfd, message, buffer, host, port);
    }
  } else {  // else loop_cnt times
    for(i = 0; i < loop_cnt; i++) { // Lähetetään loop_cnt verran
      sendAndReceive(sockfd, message, buffer, host, port);
    }
  }
  // close socket
  close(sockfd);  
  return 0;
}

// send and receive func
void sendAndReceive(int sockfd, const char* message, char* buffer, const char* host, int port) {
  send(sockfd, message, strlen(message), 0);  
    printf("Message to %s:%d sent: %s\n", host, port, message);

    int n = recv(sockfd, buffer, MAXLINE-1, 0);  
    if(n < 0){  
        perror("Recv failed..exiting\n");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0'; 
    printf("Server returned: %s\n", buffer); 
}

