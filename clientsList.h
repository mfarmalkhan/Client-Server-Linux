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

#define TRUE 1
#define FALSE 0
typedef int BOOL;

typedef struct _client 
{ 
	char *ip;
	struct tm s_time;
    //struct tm e_time;
    struct tm elapsed;
	int msgsock;
    int port;
    int readpipe;
    int writepipe;
	struct _client *next;
} client;

client *newclient(char *ip, int msgsock,int port,struct tm s_time, struct tm elapsed,int readpipe, int writepipe) {

        client *newp;
        newp = (client *) malloc (sizeof(client));
        newp -> ip = (char *)malloc(strlen(ip) + 1);
		newp -> msgsock = msgsock; 
		//newp ->processTime = processTime;
        newp -> s_time = s_time;
        //newp -> e_time = e_time;
        newp -> elapsed = elapsed;
        newp -> port = port;
        newp -> readpipe = readpipe;
        newp ->writepipe = writepipe;

        strcpy(newp -> ip, ip);
        newp -> next = NULL;
        return newp;
}  

void insertclient(client **head, client *newp)
{
	client **tracer = head;

	while((*tracer) && 
		  strcmp((*tracer)->ip, newp -> ip) < 1)
		  //(*tracer) -> pid < newp -> pid)
	{
		tracer = &(*tracer)->next;
	}

	newp -> next = *tracer;
	*tracer = newp;
}

void delclient(client **head, char* ip)
{
	BOOL present = FALSE;
	client *old;
	client **tracer = head;
	
	while((*tracer) && (strcmp(ip,(*tracer)->ip) != 0)) {
		tracer = &(*tracer)->next;
	}
		old = *tracer;
		*tracer = (*tracer)->next;
        
		free(old -> ip); // free off space used by text string
		free(old); // free up remainder of list element 

}

void printClients(client **head,char temp[]) {
	//char *buff = (char*) malloc (1000 * sizeof(char));
    char buff[1000];
	client **tracer = head;
	while (*tracer) {
		//int x = sprintf(buff,"%s %u\n",(*tracer)->processName,(*tracer)-> pid);
		int x = sprintf(buff,"ip address: %s | port: %d | msg_sock: %d | start time: %02d:%02d:%02d\n",(*tracer)-> ip, (*tracer) -> port ,(*tracer)->msgsock, 
        (*tracer) ->s_time.tm_hour,(*tracer) -> s_time.tm_min,(*tracer) -> s_time.tm_sec);

		strncat(temp,buff,x);		
		tracer = &(*tracer)->next;
	}
}


void clientTime(client **head) {
    client **tracer = head;
    struct tm elapsed;
    time_t T = time(NULL);
    struct  tm curr_time = *localtime(&T);
    time_t elapsedTime;

    while (*tracer) {

        int seconds1 = curr_time.tm_hour*60*60 + curr_time.tm_min*60 +  curr_time.tm_sec;
        int seconds2 =  ((*tracer) -> s_time).tm_hour*60*60 + ((*tracer) -> s_time).tm_min*60 +  ((*tracer) -> s_time).tm_sec;
        int totalSeconds = seconds1-seconds2;

        //extract time in Hours, Minutes and Seconds
        (*tracer) -> elapsed.tm_min  = totalSeconds/60;
        (*tracer) -> elapsed.tm_hour  = (*tracer) -> elapsed.tm_min/60;
        (*tracer) -> elapsed.tm_min   = (*tracer) -> elapsed.tm_min%60;
        (*tracer) -> elapsed.tm_sec  = totalSeconds%60;   
   
        tracer = &(*tracer)->next;
    }
}

void updateClientTime(client **head) {

	client **tracer = head;
	
	time_t T = time(NULL);
	struct tm curr_time = *localtime(&T);

	while (*tracer != NULL) {

	int seconds1 = (curr_time.tm_hour*60*60) + (curr_time.tm_min*60) +  (curr_time.tm_sec);
    int seconds2 =  ((*tracer) -> s_time).tm_hour*60*60 + ((*tracer) -> s_time).tm_min*60 +  ((*tracer) -> s_time).tm_sec;
    int totalSeconds = seconds1-seconds2;


     //extract time in Hours, Minutes and Seconds
    (*tracer) -> elapsed.tm_min  = totalSeconds/60;
    (*tracer) -> elapsed.tm_hour  = (*tracer) -> elapsed.tm_min/60;
    (*tracer) -> elapsed.tm_min   = (*tracer) -> elapsed.tm_min%60;
    (*tracer) -> elapsed.tm_sec  = totalSeconds%60;
    
    tracer = &(*tracer)->next; 
	}
}

int getClients(client **head) {

    client **tracer = head;
    int counter = 0;

    while((*tracer)!=NULL) {
        counter++;
        tracer = &(*tracer)->next;
    }
    return counter;
}