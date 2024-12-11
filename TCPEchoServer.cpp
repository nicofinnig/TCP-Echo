/* TCPEchoServer.cpp
 * 
 * Normal build
 * $ g++ -std=c++14 -pthread TCPEchoServer.cpp -o executables/TCPEchoServer
 * Run command: ./TCPEchoServer <port>
 * Example:
 * ./TCPEchoServer 8080
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
#include <pthread.h>

#define PORT 8080
#define MAX_QUEUE 3
#define MAXLINE 1024

void IntHandler(int sig);
void *SocketHandlerThread(void *); 
//long IncEchoMessageCounter(long *pcounter);

// global vars
static int server_fd = -1;  

pthread_mutex_t lock; // mutex lock
static long echo_message_counter = 0; 

int main(int argc, char *argv[]) {
  struct sockaddr_in address;
  struct sockaddr_in client_address;  
  int communication_socket; 
  pthread_t thread_id; 
  socklen_t client_address_len; 
  int port = PORT;

  // if port is given as cmd line parameter
  if(argc == 2) {           
    port = atoi(argv[1]);  
  }

  // initialize mutex
  if(pthread_mutex_init(&lock, NULL) !=0 ) {
    perror("Mutex init failed...exiting\n");
    exit(EXIT_FAILURE);
  }

  // create server socket 
  if((server_fd=socket(AF_INET, SOCK_STREAM,0)) == 0) { 
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if(bind(server_fd,(const sockaddr *)&address, sizeof(address)) < 0) {  
    perror("Bind failed (port already taken probably)\n"); 
    exit(EXIT_FAILURE);
  }

  //Listen
  if(listen(server_fd, MAX_QUEUE) < 0) {  
    perror("listen failure...exiting\n"); 
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, IntHandler); 
  printf("TCPEchoServer listening on port: %d\nPress ctrl+C to exit...\n", port);

  // Accept incoming connections
  while(true) {
    client_address_len = sizeof(client_address); 
    if((communication_socket = accept(server_fd, (sockaddr *)&client_address, (socklen_t *)&client_address_len)) < 0) { 
      perror("accept failure...exiting\n");
      exit(EXIT_FAILURE);
    }
    // create a new thread for each client
    if(pthread_create(&thread_id, NULL, SocketHandlerThread, &communication_socket) != 0) { 
      perror("Could not create client handler thread...exiting\n"); 
      exit(EXIT_FAILURE);
    }
    printf("Connection from %s:%u\n", inet_ntoa(client_address.sin_addr), client_address.sin_port);
  }
}
// Thread function
void *SocketHandlerThread(void *socket_desc) {  
  int socket = *(int *)socket_desc; 
  int read_size, loop_cnt = 0;
  long total_loop_counter;
  char rbuffer[MAXLINE];
  char wbuffer[MAXLINE+20]; // write needs some extra space for loop counter
  pthread_t tid = pthread_self();
  
  // waiting for client message
  while((read_size = recv(socket, rbuffer, sizeof(rbuffer)-1, 0)) > 0) {   
    loop_cnt++; // loop counter, every thread has its own!
    rbuffer[read_size] = '\0';  // Null-terminate the buffer to be sure
    printf("Message from client: %s\n", rbuffer);  
    //total_loop_counter = IncEchoMessageCounter(&echo_message_counter); 

    sprintf(wbuffer, "loop count: %d %s", loop_cnt, rbuffer); 
    send(socket, wbuffer, strlen(wbuffer), 0);

    memset(rbuffer, 0, MAXLINE);
  }
  if(read_size == 0) { // client has disconnected
    printf("Client disconnected...\n"); 
    fflush(stdout); 
  } else if (read_size == -1) { // -1 = error
    perror("recv failed in thread\n");
  }
  close(socket); 
  return 0;
}

void IntHandler(int sig) {
  char c;

  signal(sig, SIG_IGN); 
  printf("Do you really want to quit [y/n]: ");
  c = getchar();  
  if(c == 'y' || c =='Y') {
    close(server_fd);
    exit(0); 
  }
  else{
    signal(SIGINT,IntHandler);
  }
}

// long IncEchoMessageCounter(long *pcounter) {
//   long value_after;
//   pthread_mutex_lock(&lock);  
//   *pcounter += 1;
//   value_after = *pcounter;
//   pthread_mutex_unlock(&lock); 
//   return value_after; 
// }