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
#include "clientsList.h"
#include "linkedList.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

char* add(float numList[10000],int length) {

    char *printAns = (char*) malloc (50 * sizeof(char));
    float sum = 0;
    for (int i=0; i<length; i++) {
        sum += numList[i];
    }
    int x = sprintf(printAns,"Result: %.2f\n",sum);
    //write(STDOUT_FILENO,printAns,x);
    return printAns;
}

char* sub(float numList[10000],int length) {

    float result = numList[0];

    for(int i=1; i<length; i++) {
        result = result - numList[i];
    }
    char *printAns = (char*) malloc (50 * sizeof(char));
    int x = sprintf(printAns,"Result: %.2f\n",result);
    //write(STDOUT_FILENO,printAns,x);
    return printAns;
}

char* multiply(float numList[10000],int length) {

    float result = numList[0];

    for(int i=1; i<length; i++) {
        result = result * numList[i];
    }
    char *printAns = (char*) malloc (50 * sizeof(char));
    int x = sprintf(printAns,"Result: %.2f\n",result);
    //write(STDOUT_FILENO,printAns,x);
    return printAns;
}

char* divide(float numList[10000],int length) {

    float result = numList[0];

    for(int i=1; i<length; i++) {
        result = result/numList[i];
    }

    char *printAns = (char*) malloc (50 * sizeof(char));

    //divide by 0 check
    if (!isinf(result)) {
    int x = sprintf(printAns,"Result: %.2f\n",result);
    //write(STDOUT_FILENO,printAns,x);
    }
    else {
        int x = sprintf(printAns,"Cannot divide by 0\n");
    }
    return printAns;
}


int run(char *arr, int length) {    

    int status;
    int ret;
    char buff2[50];
    
    int fk = fork();

    if (fk == -1) {
        perror("fork");
        exit(1);
    }

    else if (fk == 0) {

        char* const args[] = {arr,NULL};

        sprintf(buff2,"/usr/bin/%s",arr);

        //ret = execv(buff2,args);
        //ret = execl(buff2,buff2,NULL);
        ret = execlp(arr,arr,NULL);
        //ret = execvp(buff,args);
    
        if (ret == -1) {
            if (errno == ENOENT) {
                write(STDOUT_FILENO,"Invalid executable, try again\n",31);
            }
            else {
                perror("exec");
                exit(1);
            }
        }
    }
}

//for process list
NODE *start = NULL;
NODE *start2 = NULL;

//for clients list
client *clientList = NULL;
int clientFd[2];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void sigHandler(int signo) {

    int status;
    int pid;

    pid = wait(&status);

    if (pid == -1) {
        perror("Unable to close window");
    }

    if (signo == SIGCHLD) {
        
        //delete entry from running list
        //delNode(&start,pid);

        if (WEXITSTATUS(status) == 250) {
            //del entry from 2nd list
            delNode(&start2,pid);
        }
        //update end time in 2nd list
        //updateTime(&start2,pid);
        updateTime(&start2,pid);
        
        
    }

    // if (signo == SIGINT) {
    //     kill(0,SIGTERM);
    // }
}

void *userInput(void *ptr);

void *manageClient(void *ptr);

int main(int argc, char **argv) {

    int fd[2];
    int cpid;
    char *processName;
    char buff[60000];
    char arr[60000];
  

    //signals defined
    __sighandler_t serr;   
    serr = signal(SIGCHLD,sigHandler);

    if (serr == SIG_ERR) {
        exit(1);
    }

    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    //socket for server created
 
    struct sockaddr_in server_address;
    

    server_address.sin_family=AF_INET;
    server_address.sin_port = 0;
    server_address.sin_addr.s_addr=INADDR_ANY;

    int binding;
    binding = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    //binding the server socket to the ports

    if (binding < 0) {
        perror("binding socket");
        exit(1);
    }

    int length = sizeof(server_address);

	if (getsockname(server_socket, (struct sockaddr *) &server_address, (socklen_t*) &length)) {
		perror("getting socket name");
		exit(1);
	}
    char portbuff[100];
	int p = sprintf(portbuff,"Socket has port #%d\n", ntohs(server_address.sin_port));
    write(STDOUT_FILENO,portbuff,p);
	fflush(stdout);

    listen(server_socket,5);
    //listen for client sockets
 
    int client_socket;
    int multiPid;

    int firstclient = 0;

    //thread for taking input in server
    int *sock = &client_socket;
    pthread_t c_thread;
    int ret = pthread_create(&c_thread, NULL, userInput, (void*) sock);

    if(ret == -1) {
        perror("error creating thread");
        exit(1);
    }
     
    while(1){

        struct sockaddr_in new_address;
        socklen_t addr_size = sizeof(new_address);

        client_socket = accept(server_socket,(struct sockaddr*)&new_address, &addr_size);
        //accepting a connection from client socket
        //int res = getpeername(client_socket,(struct sockaddr*)&new_address, &addr_size);
      
        if (client_socket < 0) {
          exit(1);
        }

        char *ipaddress = new char[20];
        strcpy(ipaddress,inet_ntoa(new_address.sin_addr));
        int portno = new_address.sin_port;

        //pipe for communication between server and client handler
        int clientHandlerFd[2];

        int serverCom = pipe(clientHandlerFd);

        if (serverCom == -1) {
            perror("pipe");
        }
         
        //insert client in list
        pthread_mutex_lock(&lock);
        time_t T = time(NULL);
	    struct tm curr_time = *localtime(&T);
        client *curr = newclient(ipaddress,client_socket,portno,curr_time,{0},clientHandlerFd[0],clientHandlerFd[1]);
        insertclient(&clientList,curr);
        pthread_mutex_unlock(&lock);
 
        char conBuff[100];
        //int count = sprintf(conBuff,"Connection accepted from %s:%d\n", inet_ntoa(new_address.sin_addr), ntohs(new_address.sin_port));
        int count = sprintf(conBuff,"Connection accepted with %s:%d\n",ipaddress, portno);
        write(STDOUT_FILENO,conBuff,count);

        //forking for multi client
        multiPid = fork();

        if (multiPid == -1) {
            perror("Cannot create client(fork)");
            exit(1);
        }

        //main server
        if (multiPid > 0) {
            // int *readpipe = &clientHandlerFd[0];
            // pthread_t cthread;
            // int a = pthread_create(&cthread,NULL,manageClient,(void*) readpipe);
        }

        if (multiPid == 0) {
            //close(server_socket);

            while(1) {
            
            //get data from client
            int rval = read(client_socket,buff,sizeof(buff));

            if (rval == -1) {
                perror("Read from socket");
            }
            else if (rval == 0) {
                char disconn[100];
                int d = sprintf(disconn,"Disconnected from %s\n", inet_ntoa(new_address.sin_addr));
                write(clientHandlerFd[1],ipaddress,strlen(ipaddress));
                write(STDOUT_FILENO,disconn,d);
                break;
            }
            else {

        strcpy(arr,buff);

        char *token;

        token = strtok(arr," \n");

        if (strcmp(token,"add") == 0) {
            float numList[10000];
            int i=0;
            char *ans = (char*) malloc (sizeof(char));
            token = strtok(NULL," ");
            while (token != NULL) {
                numList[i] = atof(token);
                i++;
                token = strtok(NULL," ");
            }
           
            ans = add(numList,i);
             for(int j=0; j<i; j++) {
                 numList[j] = 0;
            }      
            write(client_socket,ans,strlen(ans));
            memset(ans,0,sizeof(ans));
        }
        else if (strcmp(token,"mul") == 0) {
            float numList[10000];
            int i=0;
            char *ans = (char*) malloc (sizeof(char));
            token = strtok(NULL," ");
            while (token != NULL) {
                numList[i] = atof(token);
                i++;
                token = strtok(NULL," ");
            }
            ans = multiply(numList,i);
            write(client_socket,ans,strlen(ans));
            memset(ans,0,sizeof(ans));
        }
        else if (strcmp(token,"div") == 0) {
            float numList[10000];
            int i=0;
            char *ans = (char*) malloc (sizeof(char));
            token = strtok(NULL," ");
            while (token != NULL) {
                numList[i] = atof(token);
                i++;
                token = strtok(NULL," ");
            }
            ans = divide(numList,i);
            write(client_socket,ans,strlen(ans));
            memset(ans,0,sizeof(ans));
        }
        else if (strcmp(token,"sub") == 0) {
            float numList[10000];
            int i=0;
            char *ans = (char*) malloc (sizeof(char));
            token = strtok(NULL," ");
            while (token != NULL) {
                numList[i] = atof(token);
                i++;
                token = strtok(NULL," ");
            }
            ans = sub(numList,i);
            write(client_socket,ans,strlen(ans));
            memset(ans,0,sizeof(ans));
        }
        else if(strcmp(token,"run") == 0) {
            token = strtok(NULL," \n");
            int length = strlen(token);
            //run(token,length);
            int ret;
            char buff2[50];
            int fd[2];

            int p = pipe2(fd,__O_CLOEXEC);

            if (p == -1) {
                perror("list pipe");
            }

            int fk = fork();

            if (fk == -1) {
                perror("fork");
                exit(1);
            }

            else if (fk > 0) {
           
                time_t T = time(NULL);
                struct  tm s_time = *localtime(&T);

                cpid = fk;
                processName = token;
                
                close(fd[1]);
                char checkExec[100];
                int y = read(fd[0], checkExec, 100);

                if(y == 0){
                    NODE *c_list = newNode(token,fk,s_time,{0},{0});
                    NODE *a_list = newNode(token,fk,s_time,{0},{0});
                    insertNode(&start,c_list);
                    insertNode(&start2,a_list);
                    write(client_socket, "Program opened\n", 16);
                }
                else if (y > 0) {
                    //delNode(&start,cpid);
                    //delNode(&start2,cpid);

                    if (strncmp(checkExec,"2",y) == 0) {
                        write(client_socket,"Invalid program name, try again\n",33);
                    }
                    else {
                        perror("exec");
                    }
                }
            }

            else if (fk == 0) {
                
            char* const args[] = {token,NULL};

            sprintf(buff2,"/usr/bin/%s",token);

            //ret = execv(buff2,args);
            //ret = execl(buff2,buff2,NULL);
            ret = execlp(token,token,NULL);
            //ret = execvp(buff,args);

            char e[10];
            int s = sprintf(e, "%d", errno);
            write(fd[1], e, s);
            exit(0); 
    
            }
        }
        else if(strcmp(token,"exit") == 0) {
            write(client_socket,"All programs terminated\n",25);

            //kill all running processes(children)
            NODE **tracer = &start;
            int currpid;
            while(*tracer) {
                currpid = (*tracer) -> pid;
                kill(currpid,SIGTERM);
                tracer = &(*tracer) -> next;
            }
            //break;
        }
        else if(strcmp(token,"disconnect") == 0) {

            //delete client from list
            write(clientHandlerFd[1],ipaddress,strlen(ipaddress));

            char disconn[100];
            int d = sprintf(disconn,"Disconnected from %s\n", inet_ntoa(new_address.sin_addr));
            write(STDOUT_FILENO,disconn,d);
            write(client_socket,"Disconnect",11);
            break;
        }
        else if (strcmp(token,"close") == 0) {
            token = strtok(NULL," \n");
            int length = strlen(token);
            int pid = 2;
            pid = atoi(token);

            if (pid == 0) {
                write(client_socket,"please enter a program pid\n",28);
            }
            else {
                int k = kill(pid,SIGTERM);

                if (k == -1) {
                    if (errno = ESRCH) {
                        write(client_socket,"please enter a valid pid\n",26);
                    }
                    else {
                        perror("unable to close program");
                        //exit(1);
                    }
                }
                else {
                    write(client_socket,"program closed\n",16);
                    
                    delNode(&start,pid);

                    updateTime(&start2,pid);
                    //write(STDOUT_FILENO,e_time,len);
                
                }
            }
        }
        else if (strcmp(token,"list") == 0) {
             if (start2 == NULL) {
                write(client_socket,"NO processes\n",14);    
            }
            else {
                elapsedTime(&start2);
                char finalList[10000];
                //write(client_socket,"All processes:\n",16);
                printAll(&start2,finalList);
                write(client_socket,finalList,strlen(finalList));
                strcpy(finalList,"");
            }
        }
        else {
            write(client_socket,"Invalid command or syntax, try again\n",38);
        }
    }
    memset(arr,0,sizeof(arr));
    //memset(buff,0,sizeof(buff));
    }
    }
    }
    //close(server_socket);
    return 0;
}

void *userInput(void *ptr) {

    char takeInput[60000];

    while (1) {
      
        int readInput = read(STDIN_FILENO,takeInput,sizeof(takeInput));

        if (readInput == 60000) {
            write(STDOUT_FILENO,"Input limit exceeded\n",22);
        }
        else {

        if (strncmp(takeInput,"list\n",5) == 0) {
            
            if (clientList != NULL){
                updateClientTime(&clientList);
                char temp[1000];
                printClients(&clientList,temp);
                write(STDOUT_FILENO,temp,strlen(temp));
                memset(temp,0,sizeof(temp));
            }
            else {
                write(STDOUT_FILENO,"No clients connected\n",22);
            }
        }
        else if (strncmp(takeInput,"exit\n",5) == 0) {
             //traverse clients list
                client **tracer = &clientList;
                int sock;
                while((*tracer) != NULL) {
                    sock = (*tracer)->msgsock;
                    //write(sock,"exit",5); 
                    close(sock);
                    tracer = &(*tracer)->next;
                }
            kill(0,SIGTERM);
        }

        else if(strncmp(takeInput,"remove ",7) == 0) {
            char sub[readInput - 8];
            //char *sub = (char*) malloc (readInput - 7);
            strncpy(sub,takeInput + 7,readInput);
            int foundip = 0;
                client **tracer = &clientList;
                char *ip = sub;
                int sock;
                sub[sizeof(sub)] = '\0';
                // write(STDOUT_FILENO,sub,strlen(sub));
                // write(STDOUT_FILENO,sub,strlen(sub));
                while((*tracer) != NULL) {
                    if(strcmp(ip,(*tracer)->ip) == 0) {
                        foundip = 1;
                        sock = (*tracer)->msgsock; 
                        int output = write(sock,"Disconnect",11);
                        pthread_mutex_lock(&lock);
                        delclient(&clientList,ip);
                        pthread_mutex_unlock(&lock);
                        //write(STDOUT_FILENO,socktakeInput,t);
                        close(sock);
                        if (output == -1) {
                            perror("server cannot communicate with client");
                        }
                    break;
                    }
                    else {
                        tracer = &(*tracer)->next;
                }
                if (foundip == 0) {
                    write(STDOUT_FILENO,"please provide valid ip\n",25);
                }
        }
        }
        else if (strncmp(takeInput,"print ",6) == 0) {
                char sub[readInput - 7];
                strncpy(sub,takeInput + 6,readInput);
                //write(STDOUT_FILENO,sub,strlen(sub));
                int i;
                int foundip = 0;
                bool flag = FALSE;

                for(i = 0; i < sizeof(sub); i++){
                    if(sub[i] == ' '){
                        flag = TRUE;    
                        break;
                    }
                }

                if (!flag) {
                    write(STDOUT_FILENO,"Invalid syntax of print message\n",32);
                }
                else {
                    char ip[i];
                    int c = 0;

                    while (c<sizeof(ip)) {
                        ip[c] = sub[c];
                        c++;
                    }
                    //ip[c+1] = '\0'
                    char *finalip = ip;
                    // write(STDOUT_FILENO,ip,i);
                    // write(STDOUT_FILENO,ip,i);
                    char msg[1000];
                    int k=0;

                    while(sub[i+1] != '\n') {
                        msg[k] = sub[i+1];
                        i++;
                        k++;
                    }
                    client **tracer = &clientList;
                    int sock;
                    char finalmessage[1000];

                    while((*tracer)!=NULL) {
                        if(strcmp(finalip,(*tracer)->ip) == 0) {
                            sock = (*tracer)->msgsock;
                            foundip = 1;
                            break;
                        }
                        else {
                            tracer = &(*tracer)->next;
                        }
                    }
                    if (foundip == 1) {
                        char finalmsg[1000];
                        int x = sprintf(finalmsg,"[SERVER]: %s\n",msg);
                        int o = write(sock,finalmsg,x);

                        if(o == -1) {
                            write(STDOUT_FILENO,"Message not sent\n",17);
                        }
                        else {
                            write(STDOUT_FILENO,"Message sent\n",13);
                        }
                    }
                    else {
                        write(STDOUT_FILENO,"IP not found\n",13);
                    }
                }
        }
        else if (strncmp(takeInput,"printall ",9) == 0) {

            char sub[readInput - 10];
            strncpy(sub,takeInput + 9,readInput);

                client **tracer = &clientList;
                int sock;
                char finalmessage[1000];
                while((*tracer) != NULL) {
                    sock = (*tracer)->msgsock;
                    int x = sprintf(finalmessage,"[SERVER]: %s",sub); 
                    int output = write(sock,finalmessage,x);
                    //write(STDOUT_FILENO,socktakeInput,t);
                    if (output == -1) {
                         perror("server cannot communicate with all clients");
                    }
                    else {
                        write(STDOUT_FILENO,"Message sent!\n",14);
                    }
                    tracer = &(*tracer)->next;
                }
               
        }
        else if(strncmp(takeInput,"count\n",6) == 0) {
            //pthread_mutex_lock(&lock);
            char number[50];
            int count = getClients(&clientList);
            int y = sprintf(number,"Number of clients: %d\n",count);
            write(STDOUT_FILENO,number,y);
            //pthread_mutex_unlock(&lock);

        }
        
        else {
            write(STDOUT_FILENO,"Invalid command\n",17);
            //write(STDOUT_FILENO,buff,strlen(buff));
        }
        memset(takeInput,0,sizeof(takeInput));
    }
    }
}

void *manageClient(void *ptr) {

    while (1) {

    client **tracer = &clientList;
    int pipefd = *((int*)ptr);

    char ipArr[20];

    int r = read(pipefd,ipArr,20);

    pthread_mutex_lock(&lock);

    delclient(&clientList,ipArr);

    pthread_mutex_unlock(&lock);

    }
}
