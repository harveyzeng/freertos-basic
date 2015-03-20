#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"
#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;
static xTaskHandle xHandle1;
static xTaskHandle xHandle2;
static xTaskHandle xHandle3;
void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
void tasktest_command(int,char **);
void _command(int, char **);
void new_command(int,char**);
#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}
extern int cmdup[50];
extern int cmduptimes;
cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	MKCL(tasktest,"test  task works"),
	MKCL(, ""),
	MKCL(new,"create new task")
	
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    	fio_printf(1,"\r\n"); 
 

    	char path[20]="";
	strcpy(path, "/romfs/");		
       
    	int dir;
    	if(n == 1){
       		 dir = fs_opendir(path);
		
    	}else if(n == 2){
		 strcat(path, argv[1]) ;
        	dir = fs_opendir(path);
	
        	//if(dir == )
    	}else{ 
        	fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);

        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");	
	fio_printf(1, "%s\r\n", buf );

}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}
void mytest1(void *pvParameters)
{		
	for (;;){	
		vTaskDelay(1000);
		}
}

void test1(void *pvParameters)
{	int next;
	int first=0;
	int second=1;
	int i;	
	fio_printf(1,"\r\n");	
	for ( i = 0 ; i < 46 ; i++ ){
      		if ( i <= 1 )
         		next = i;
      		else{
         		next = first + second;
         		first = second;
         		second = next;
      			}
      	
   	}
fio_printf(1,"\r\ntest 1 finished 36th fibonacci:%d\r\n",next);

vTaskDelete( xHandle1 );

}

void test2(void *pvParameters)
{	int next;
	int first=0;
	int second=1;
	int i;	
	fio_printf(1,"\r\n");	
	for ( i = 0 ; i < 46 ; i++ ){
      		if ( i <= 1 )
         		next = i;
      		else{
         		next = first + second;
         		first = second;
         		second = next;
      			}
      	
   	}
fio_printf(1,"\r\ntest 2 finished 36th fibonacci:%d\r\n",next);

 vTaskDelete( xHandle2 );

}
void test3(void *pvParameters)
{	int next;
	int first=0;
	int second=1;
	int i;	
	fio_printf(1,"\r\n");	
	for ( i = 0 ; i < 46 ; i++ ){
      		if ( i <= 1 )
         		next = i;
      		else{
         		next = first + second;
         		first = second;
         		second = next;
      			}
      	
   	}
fio_printf(1,"\r\ntest 3 finished 36th fibonacci:%d\r\n",next);

 vTaskDelete( xHandle3 );

}

int StrToInt( char *str)
{
    int n = 0;
    while (*str != 0)
    {
        int c = *str - '0';
        n = n * 10 + c;
        ++str;
    }
    return n;
}
void new_command(int n, char *argv[]){
/* Create a task to mytest1. */
	if(n==1){
		fio_printf(2, "\r\nUsage: new <newtaskname> <priority>\r\n");
		return;
	}
	char *name=argv[1];
	int input=StrToInt(argv[2]);
	if(xTaskCreate(mytest1,
	            (signed portCHAR *) name,
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + input, NULL)!=-1){

        fio_printf(2, "\r\ncreate new task:%s '\r\n",name);}
else 
	fio_printf(2, "\r\ncan't create task! '\r\n");
    
} 
void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void tasktest_command(int n, char *argv[]) {
   	if(n==1){
		fio_printf(2, "\r\nUsage: tasktest <task1priority> <task2priority> <task3priority>\r\n");
return;
}
	int task1priority=StrToInt(argv[1]);
	int task2priority=StrToInt(argv[2]);
	int task3priority=StrToInt(argv[3]);
	 // Create the task, storing the handle.
	 xTaskCreate( test1,(signed portCHAR *)"name", 128, NULL, task1priority, &xHandle1 );

 xTaskCreate( test2, (signed portCHAR *)"name2", 128, NULL, task2priority, &xHandle2 );
	 
xTaskCreate( test3,(signed portCHAR *)"name3", 128, NULL, task3priority, &xHandle3 );
	 

	

			
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}


cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			{cmdup[cmduptimes]=i;
			  cmduptimes+=1;
			return cl[i].fptr;}
	}
	return NULL;	
}

void test_command(int n, char *argv[]) {
   	if(n==1){
		fio_printf(2, "\r\nUsage: test <number>\r\n");
		return;
	}
	int next;
	int first=0;
	int second=1;
	int i;
	int input=StrToInt(argv[1]);
	fio_printf(1,"\r\n");	
	for ( i = 0 ; i < input ; i++ ){
      		if ( i <= 1 )
         		next = i;
      		else{
         		next = first + second;
         		first = second;
         		second = next;
      			}
      	fio_printf(1,"%d\r\n",next);
   	}
 
			
}
