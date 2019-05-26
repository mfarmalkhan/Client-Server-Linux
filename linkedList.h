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

typedef struct _node 
{ 
	char *processName;
	struct tm s_time;
    struct tm e_time;
    struct tm elapsed;
	int pid;
	struct _node *next;
} NODE;

// NODE *start = NULL;
// NODE *start2 = NULL;

NODE *newNode(char *text, int pid,struct tm s_time, struct tm e_time,struct tm elapsed) {

        NODE *newp;
        newp = (NODE *) malloc (sizeof(NODE));
        newp -> processName = (char *)malloc(strlen(text) + 1);
		newp -> pid = pid; 
		//newp ->processTime = processTime;
        newp -> s_time = s_time;
        newp -> e_time = e_time;
        newp -> elapsed = elapsed;

        strcpy(newp -> processName, text);
        newp -> next = NULL;
        return newp;
}  

void insertNode(NODE **head, NODE *newp)
{
	NODE **tracer = head;

	while((*tracer)) //&& (*tracer) -> pid < newp -> pid)
	{
		tracer = &(*tracer)->next;
	}

	newp -> next = *tracer;
	*tracer = newp;
}

void delNode(NODE **head, int text)
{
	BOOL present = FALSE;
	NODE *old;
	NODE **tracer = head;
	
	while((*tracer) && !(present = (text == (*tracer)->pid)))
		tracer = &(*tracer)->next;

	if(present)
	{
		old = *tracer;
		*tracer = (*tracer)->next;
        
		free(old -> processName); // free off space used by text string
		free(old); // free up remainder of list element 
	}
}

void printList(NODE **head,char temp[])
{
	//char *buff = (char*) malloc (1000 * sizeof(char));
    //char temp[1000];
    char buff[1000];
	NODE **tracer = head;
	while (*tracer) {
		//int x = sprintf(buff,"%s %u\n",(*tracer)->processName,(*tracer)-> pid);
		int x = sprintf(buff,"pid: %u | name: %s | start time: %02d:%02d:%02d | elapsed time: %02d:%02d:%02d\n",(*tracer)-> pid,(*tracer)->processName, 
        (*tracer) ->s_time.tm_hour,(*tracer) -> s_time.tm_min,(*tracer) -> s_time.tm_sec,
        (*tracer) ->elapsed.tm_hour,(*tracer) -> elapsed.tm_min,(*tracer) -> elapsed.tm_sec);

		//write(socket,buff,x);
        strncat(temp,buff,x);		
		tracer = &(*tracer)->next;
	}
    //return buff;
}
void printAll(NODE **head,char temp[]) {

    //char *buff = (char*) malloc (1000 * sizeof(char));
    char buff[1000];
	NODE **tracer = head;
    int x;
	while (*tracer) {
        
        if (((*tracer) -> e_time).tm_hour == 0 && ((*tracer) -> e_time).tm_min == 0 &&
            ((*tracer) -> e_time).tm_sec == 0) {
            
		x = sprintf(buff,"pid: %u | name: %s | start time: %02d:%02d:%02d | end time: %02d:%02d:%02d | elapsed time: %02d:%02d:%02d | status: OPEN \n",(*tracer)-> pid,(*tracer)->processName, 
        (*tracer) ->s_time.tm_hour,(*tracer) -> s_time.tm_min,(*tracer) -> s_time.tm_sec,
        (*tracer) ->e_time.tm_hour,(*tracer) -> e_time.tm_min,(*tracer) -> e_time.tm_sec,
        (*tracer) ->elapsed.tm_hour,(*tracer) -> elapsed.tm_min,(*tracer) -> elapsed.tm_sec);

		//write(socket,buff,x);
       
	    }
        else {
            x = sprintf(buff,"pid: %u | name: %s | start time: %02d:%02d:%02d | end time: %02d:%02d:%02d | elapsed time: %02d:%02d:%02d | status: CLOSED \n",(*tracer)-> pid,(*tracer)->processName, 
            (*tracer) ->s_time.tm_hour,(*tracer) -> s_time.tm_min,(*tracer) -> s_time.tm_sec,
            (*tracer) ->e_time.tm_hour,(*tracer) -> e_time.tm_min,(*tracer) -> e_time.tm_sec,
            (*tracer) ->elapsed.tm_hour,(*tracer) -> elapsed.tm_min,(*tracer) -> elapsed.tm_sec);

        }
        strncat(temp,buff,x);		
		tracer = &(*tracer)->next;
    //return buff;
    }
}

void updateTime(NODE **head, int pid) {
    NODE **tracer = head;

    char s_time[100];
    time_t T = time(NULL);
    struct  tm e_time = *localtime(&T);

       while(*tracer != NULL) {
           if(pid == (*tracer) ->pid) {
               (*tracer) ->e_time = e_time;
               break;
           }           
           tracer = &(*tracer)->next;
         
       }
}
void elapsedTime(NODE **head) {
    NODE **tracer = head;
    struct tm elapsed;
    time_t T = time(NULL);
    struct  tm curr_time = *localtime(&T);
    time_t elapsedTime;

    while (*tracer) {

        if (((*tracer) -> e_time).tm_hour == 0 && ((*tracer) -> e_time).tm_min == 0 &&
        ((*tracer) -> e_time).tm_sec == 0) {
            
        int seconds1 = curr_time.tm_hour*60*60 + curr_time.tm_min*60 +  curr_time.tm_sec;
        int seconds2 =  ((*tracer) -> s_time).tm_hour*60*60 + ((*tracer) -> s_time).tm_min*60 +  ((*tracer) -> s_time).tm_sec;
        int totalSeconds = seconds1-seconds2;

        //extract time in Hours, Minutes and Seconds
        (*tracer) -> elapsed.tm_min  = totalSeconds/60;
        (*tracer) -> elapsed.tm_hour  = (*tracer) -> elapsed.tm_min/60;
        (*tracer) -> elapsed.tm_min   = (*tracer) -> elapsed.tm_min%60;
        (*tracer) -> elapsed.tm_sec  = totalSeconds%60;   
    }
    else {

        // elapsedTime = (time_t) difftime(mktime(&((*tracer) -> e_time)),mktime(&((*tracer) -> s_time)));
        // elapsed = *localtime(&elapsedTime);

    int seconds1 = ((*tracer) -> e_time).tm_hour*60*60 + ((*tracer) -> e_time).tm_min*60 +  ((*tracer) -> e_time).tm_sec;
    int seconds2 =  ((*tracer) -> s_time).tm_hour*60*60 + ((*tracer) -> s_time).tm_min*60 +  ((*tracer) -> s_time).tm_sec;
    int totalSeconds = seconds1-seconds2;

     //extract time in Hours, Minutes and Seconds
    (*tracer) -> elapsed.tm_min  = totalSeconds/60;
    (*tracer) -> elapsed.tm_hour  = (*tracer) -> elapsed.tm_min/60;
    (*tracer) -> elapsed.tm_min   = (*tracer) -> elapsed.tm_min%60;
    (*tracer) -> elapsed.tm_sec  = totalSeconds%60;
    }
    tracer = &(*tracer)->next;
    }
}

