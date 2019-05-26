#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "linkedList.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

struct thread_args
{
    char *input;
    int sock;
    int length;
};


void *sendCommand(void *ptr);

void *receiveMsg(void *ptr);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        write(STDOUT_FILENO,"Please provide IP and port #\n",30);
        exit(EXIT_FAILURE);
    }
    int network_socket;
    network_socket=socket(AF_INET, SOCK_STREAM, 0);
    //socket created

    //multithreading
    pthread_t thread1;
    pthread_t thread2;

    struct sockaddr_in server_address;
    struct hostent *hp;
    in_addr_t data; 
    
    server_address.sin_family=AF_INET;

    //data = inet_addr(argv[1]);
    // hp = gethostbyaddr(&data, 4, AF_INET);
    hp = gethostbyname(argv[1]);

    if (hp == 0) {
	    //fprintf(stderr, "%s: unknown host\n", argv[1]);
        write(STDOUT_FILENO,"unknown host\n",14);
	 	exit(2);
	}
    
    //inet_pton(AF_INET,argv[1],&server_address.sin_addr);
    
    bcopy(hp->h_addr, &server_address.sin_addr, hp->h_length);

    server_address.sin_port= htons(atoi(argv[2]));
    //server_address.sin_addr.s_addr=inet_aton(argv[1],&server_address);
 
    int connect_socket = connect(network_socket,(struct sockaddr*)&server_address,sizeof(server_address));
    
    if(connect_socket == -1)
    {
        perror("connecting to socket");
        exit(1);
    }
    else {
        write(STDOUT_FILENO,"Connection established with server\n",36);
    }
    //socket connected to the server
 
    char input[60000];
    int *sock = &network_socket;

    pthread_t c_thread;
    int ret = pthread_create(&c_thread, NULL, receiveMsg, (void*) sock);

    if(ret != 0){
        perror("thread");
        exit(1);
    }

    //recieve data from server
    while(1) {

    int size = read(STDIN_FILENO,input,60000);

    if (size == 60000) {
        write(STDOUT_FILENO,"The input size has exceeded the limit\n",38);
    }

    else {
    
    int output = write(network_socket,input,size);

    if (output == -1) {
        write(STDOUT_FILENO,"Unable to send data\n",21);
    }


    }
    }
    //close(sock);
    return 0;
}   

//getting input from server
void *receiveMsg(void *sock) {

    int network_socket = *((int*)sock);
    char msg[60000];

    while(1) {

    int size = read(network_socket,msg,sizeof(msg));

    if (size == -1) {
        perror("Unable to receive data from server");
    }

    if (size == 0) {
        write(STDOUT_FILENO,"The server has closed the connection\n",38);
        exit(0);
    }

    if (strncmp(msg,"Disconnect",11) == 0) {
        write(STDOUT_FILENO,"Disconnected from server\n",26);
        exit(0);
    }
    else {
        int output = write(STDOUT_FILENO,msg,size);
    }
    }
    memset(msg,0,sizeof(msg));

}

