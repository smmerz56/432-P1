#include <iostream>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>     // gethostbyname
#include <unistd.h>    // read, write, close
#include <strings.h>     // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <pthread.h>      // pthreads
#include <sys/time.h>      //gettimeofday
using namespace std;

//-----------------------------------------------------------------------------
// Server: bind, listen, and accept
//
// Notes: On my machine I had to compile every time before running***********
//-----------------------------------------------------------------------------

const unsigned int BUF_SIZE = 1500;
const unsigned int num_arguments = 3;

void *your_function(void *arg);

struct mystruct {
  int sd;
  int repetitions;
};

int main (int argc, char *argv[num_arguments]) {

    if (argc != 3){
      cerr << "Incorrect amount of arguments, try again" << endl; 
      exit(1);
    }
      
    // Create the socket
    int server_port = atoi(argv[1]);
    int repetition = atoi(argv[2]);

    sockaddr_in acceptSock;// the configuration setup
    bzero((char*) &acceptSock, sizeof(acceptSock));//zero out the structure
    
    acceptSock.sin_family = AF_INET;   // using IP
    // listen on any address this computer has
    acceptSock.sin_addr.s_addr = htonl(INADDR_ANY); 
    acceptSock.sin_port = htons(server_port);  // set the port to listen on
    
    // creates a new socket for IP using TCP 
    int serverSd = socket(AF_INET, SOCK_STREAM, 0); 

    const int on = 1;
    // this lets us reuse the socket without waiting for hte OS to recycle it
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));
    
    // Bind the socket
    bind(serverSd, (sockaddr*) &acceptSock, sizeof(acceptSock));  
    
    // Listen on the socket
    int n = 5; //if it reaches 5 connections, it starts rejecting them
    listen(serverSd, n);// listen on the socket and allow up to n connections

    // Accept the connection as a new socket    
    sockaddr_in newsock;// place to store parameters for the new connection
    socklen_t newsockSize = sizeof(newsock);
   
    while (1) {// looping infinitly looking for connections
     
     // grabs the new connection and assigns it a temporary socket 
     int newSd = accept(serverSd, (sockaddr *)&newsock, &newsockSize);
     
     struct mystruct struct1;   
     struct1.sd = newSd;
     struct1.repetitions = repetition;
       
      //create new thread
      pthread_t worker_thread;
      pthread_create(&worker_thread, NULL, your_function, (void *)&struct1);
      pthread_join(worker_thread, NULL);
    
      exit(0);
    }
    return 0;

}

//-----------------------------------------------------------------------------
// your_function
// called by the worker thread created in main
// reads in data from client
// keeps track of how long it takes to recieve data
// sends the amount of reads back to client
void *your_function(void *arg){
  char databuf[BUF_SIZE];
  struct timeval start_time, stop_time; //beginning and end timestamps
  gettimeofday(&start_time, NULL); //beginning timestamp
  
  struct mystruct struct2 = *(struct mystruct*)arg;
  
  int sd = struct2.sd;
  int repetition = struct2.repetitions;
  int count = 0;
  //int read = 0;
  for(int i = 0; i < repetition; i++){
    for ( int nRead = 0; 
          ( nRead += read( sd, databuf, BUF_SIZE - nRead ) ) < BUF_SIZE;  
          ++count ); 
      count++;
      //read++;
  }
            
  gettimeofday(&stop_time, NULL); //end timestamp
  
  unsigned long long data_receiving_time = 
          (((stop_time.tv_sec - start_time.tv_sec) * 1000000) + 
          (stop_time.tv_usec - start_time.tv_usec));
  
  write(sd, &count, sizeof(count));//write amount of reads back to the client
  
  cout << "Data-recieving time = " << data_receiving_time << " usec" << endl;
 
  close(sd);
  
  return nullptr;
}
