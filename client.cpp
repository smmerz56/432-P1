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
#include <cstring>
#include <sys/time.h>    //gettimeofday
using namespace std;

//-----------------------------------------------------------------------------
// Client: create Socket and connect 
//-----------------------------------------------------------------------------

const unsigned int num_arguments = 7;

int main (int argc, char *argv[num_arguments]) {

  if (argc < 7){
    cerr << "Too few arguements, try again" << endl;  
    exit(1);
  }

  int server_port = atoi(argv[1]); //a server IP port
  int repetition = atoi(argv[2]);//times sending a set of data buffers
  int nbufs = atoi(argv[3]); //the number of data buffers
  int bufsize = atoi(argv[4]); //the size of each data buffer (in bytes)
  char* serverlp = argv[5]; //a server IP name, converted below
  int type = atoi(argv[6]); //the type of transfer scenario: 1, 2, or 3       

  if(type != 1 && type != 2 && type != 3){
    cerr << "Incorrect type #, try again" << endl;  
    exit(1);
  }
  
  struct hostent* host = gethostbyname(serverlp);//host name
  
  sockaddr_in sendSockAddr;
  bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
  
  sendSockAddr.sin_family = AF_INET;// using IP
  sendSockAddr.sin_addr.s_addr = 
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
  sendSockAddr.sin_port = htons(server_port);
  
  int clientSd = socket(AF_INET, SOCK_STREAM, 0);
  
  int connectStatus = 
    connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
  
  if(connectStatus < 0) {
    cerr << "failed to connect to the server" << endl;
  }

  char databuf[nbufs][bufsize];//data buffers based on passed in args
  memset(databuf, 's', sizeof(databuf[0][0]) * nbufs * bufsize);
  
  //beginning, lap, and end timestamps
  struct timeval start_time, lap_time, stop_time;
  gettimeofday(&start_time, NULL); //beginning timestamp
  
  cout << repetition << " repetitions of type " << type << ":" ;
  
  switch(type){
    case 1: {
            cout << " multiple writes" << endl;
            break;
            }
    case 2: {
            cout << " writev" << endl;
            break;
            }
    case 3: 
            cout << " single write" << endl;
            break;
  }
  
  for(int i = 0; i < repetition; i++){
    switch(type){ 
      case 1: {  
                //write call for each data buffer               
                for(int j = 0; j < nbufs; j++){
                  write(clientSd, databuf[j], bufsize);
                }    
                break;
              }
      case 2: {    
                //allocates an array of iovec data structures            
                struct iovec vector[nbufs];
                for ( int j = 0; j < nbufs; j++ ) {
                  vector[j].iov_base = databuf[j];
                  vector[j].iov_len = bufsize;
                }
                writev(clientSd, vector, nbufs);           
                break;
              }
      case 3:           
              //sends databuf once    
              write(clientSd, databuf, nbufs * bufsize); 
              break;
    }
  }
 
  gettimeofday(&lap_time, NULL); //lap timestamp
  
  unsigned long long data_sending_time = 
    (((lap_time.tv_sec - start_time.tv_sec) * 1000000) + 
    (lap_time.tv_usec - start_time.tv_usec));
  
  int numReads; 
  int bytesRead = read(clientSd, &numReads, sizeof(numReads));
  
  gettimeofday(&stop_time, NULL); //end timestamp
  
  unsigned long long round_trip_time = 
    (((stop_time.tv_sec - start_time.tv_sec) * 1000000) + 
    (stop_time.tv_usec - start_time.tv_usec));
  
  cout << "Test 1: data-sending time = " << data_sending_time << 
    " usec, round-trip time = " << round_trip_time << 
    " usec, #reads = " << numReads << endl;
  
  close(clientSd);
  
  
}