#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <unistd.h>
#include "fio.h"
#include "dir.h"
#include "filesystem.h"
#include "osdebug.h"
#include "hash-djb2.h"
#include "clib.h"
static struct fddef_t fio_fds[MAX_FDS];

/* recv_byte is define in main.c */
char recv_byte();
void send_byte(char);
enum KeyName{ESC=27, BACKSPACE=127 ,UP=33};
int cmdup[50];
int cmduptimes=0;
/* Imple */
static ssize_t stdin_read(void * opaque, void * buf, size_t count) {
    int i=0, endofline=0, last_chr_is_esc;
    char *ptrbuf=buf;
    char ch;
    int currentcmdtimes;
    while(i < count&&endofline!=1){
	ptrbuf[i]=recv_byte();
	
	
	switch(ptrbuf[i]){
		case 'A':
currentcmdtimes=cmduptimes-1;
if(cmdup[currentcmdtimes]==0){ptrbuf[i]='l';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';}
else if(cmdup[currentcmdtimes]==1){ptrbuf[i]='m';send_byte(ptrbuf[i]);++i;ptrbuf[i]='a';send_byte(ptrbuf[i]);++i;ptrbuf[i]='n';}
else if (cmdup[currentcmdtimes]==2){ptrbuf[i]='c';send_byte(ptrbuf[i]);++i;ptrbuf[i]='a';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';}
else if (cmdup[currentcmdtimes]==3){ptrbuf[i]='p';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';}
else if (cmdup[currentcmdtimes]==4){ptrbuf[i]='h';send_byte(ptrbuf[i]);++i;ptrbuf[i]='o';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';}
else if (cmdup[currentcmdtimes]==5){ptrbuf[i]='m';send_byte(ptrbuf[i]);++i;ptrbuf[i]='m';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';send_byte(ptrbuf[i]);++i;ptrbuf[i]='e';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';}
else if (cmdup[currentcmdtimes]==6){ptrbuf[i]='h';send_byte(ptrbuf[i]);++i;ptrbuf[i]='e';send_byte(ptrbuf[i]);++i;ptrbuf[i]='l';send_byte(ptrbuf[i]);++i;ptrbuf[i]='p';}
else if (cmdup[currentcmdtimes]==7){ptrbuf[i]='t';send_byte(ptrbuf[i]);++i;ptrbuf[i]='e';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';}
else if (cmdup[currentcmdtimes]==8){ptrbuf[i]='t';send_byte(ptrbuf[i]);++i;ptrbuf[i]='a';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';send_byte(ptrbuf[i]);++i;ptrbuf[i]='k';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';send_byte(ptrbuf[i]);++i;ptrbuf[i]='e';send_byte(ptrbuf[i]);++i;ptrbuf[i]='s';send_byte(ptrbuf[i]);++i;ptrbuf[i]='t';}
else if (cmdup[currentcmdtimes]==10){ptrbuf[i]='n';send_byte(ptrbuf[i]);++i;ptrbuf[i]='e';send_byte(ptrbuf[i]);++i;ptrbuf[i]='w';}
currentcmdtimes-=1;
		break;	
		case '\r':
		case '\n':
			ptrbuf[i]='\0';
			endofline=1;
			break;
		case '[':
			if(last_chr_is_esc){
				last_chr_is_esc=0;
				ch=recv_byte();
				if(ch>=1&&ch<=6){
					ch=recv_byte();
				}
				continue;
			}
		case ESC:
			last_chr_is_esc=1;
			continue;
		case BACKSPACE:
			last_chr_is_esc=0;
			if(i>0){
				send_byte('\b');
				send_byte(' ');
				send_byte('\b');
				--i;
			}
			continue;
		default:
			last_chr_is_esc=0;
	}
	send_byte(ptrbuf[i]);
	++i;
    }
    return i;
}

static ssize_t stdout_write(void * opaque, const void * buf, size_t count) {
    int i;
    const char * data = (const char *) buf;
    
    for (i = 0; i < count; i++)
        send_byte(data[i]);
    
    return count;
}

static xSemaphoreHandle fio_sem = NULL;

__attribute__((constructor)) void fio_init() {
    memset(fio_fds, 0, sizeof(fio_fds));
    fio_fds[0].fdread = stdin_read;
    fio_fds[1].fdwrite = stdout_write;
    fio_fds[2].fdwrite = stdout_write;
    fio_sem = xSemaphoreCreateMutex();
}

struct fddef_t * fio_getfd(int fd) {
    if ((fd < 0) || (fd >= MAX_FDS))
        return NULL;
    return fio_fds + fd;
}

static int fio_is_open_int(int fd) {
    if ((fd < 0) || (fd >= MAX_FDS))
        return 0;
    int r = !((fio_fds[fd].fdread == NULL) &&
              (fio_fds[fd].fdwrite == NULL) &&
              (fio_fds[fd].fdseek == NULL) &&
              (fio_fds[fd].fdclose == NULL) &&
              (fio_fds[fd].opaque == NULL));
    return r;
}

static int fio_findfd() {
    int i;
    
    for (i = 0; i < MAX_FDS; i++) {
        if (!fio_is_open_int(i))
            return i;
    }
    
    return -1;
}

int fio_is_open(int fd) {
    int r = 0;
    xSemaphoreTake(fio_sem, portMAX_DELAY);
    r = fio_is_open_int(fd);
    xSemaphoreGive(fio_sem);
    return r;
}

int fio_open(fdread_t fdread, fdwrite_t fdwrite, fdseek_t fdseek, fdclose_t fdclose, void * opaque) {
    int fd;
//    DBGOUT("fio_open(%p, %p, %p, %p, %p)\r\n", fdread, fdwrite, fdseek, fdclose, opaque);
    xSemaphoreTake(fio_sem, portMAX_DELAY);
    fd = fio_findfd();
    
    if (fd >= 0) {
        fio_fds[fd].fdread = fdread;
        fio_fds[fd].fdwrite = fdwrite;
        fio_fds[fd].fdseek = fdseek;
        fio_fds[fd].fdclose = fdclose;
        fio_fds[fd].opaque = opaque;
    }
    xSemaphoreGive(fio_sem);
    
    return fd;
}

ssize_t fio_read(int fd, void * buf, size_t count) {
    ssize_t r = 0;
//    DBGOUT("fio_read(%i, %p, %i)\r\n", fd, buf, count);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdread) {
            r = fio_fds[fd].fdread(fio_fds[fd].opaque, buf, count);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

ssize_t fio_write(int fd, const void * buf, size_t count) {
    ssize_t r = 0;
//    DBGOUT("fio_write(%i, %p, %i)\r\n", fd, buf, count);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdwrite) {
            r = fio_fds[fd].fdwrite(fio_fds[fd].opaque, buf, count);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

off_t fio_seek(int fd, off_t offset, int whence) {
    off_t r = 0;
//    DBGOUT("fio_seek(%i, %i, %i)\r\n", fd, offset, whence);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdseek) {
            r = fio_fds[fd].fdseek(fio_fds[fd].opaque, offset, whence);
        } else {
            r = -3;
        }
    } else {
        r = -2;
    }
    return r;
}

int fio_close(int fd) {
    int r = 0;
//    DBGOUT("fio_close(%i)\r\n", fd);
    if (fio_is_open_int(fd)) {
        if (fio_fds[fd].fdclose)
            r = fio_fds[fd].fdclose(fio_fds[fd].opaque);
        xSemaphoreTake(fio_sem, portMAX_DELAY);
        memset(fio_fds + fd, 0, sizeof(struct fddef_t));
        xSemaphoreGive(fio_sem);
    } else {
        r = -2;
    }
    return r;
}

void fio_set_opaque(int fd, void * opaque) {
    if (fio_is_open_int(fd))
        fio_fds[fd].opaque = opaque;
}

#define stdin_hash 0x0BA00421
#define stdout_hash 0x7FA08308
#define stderr_hash 0x7FA058A3

static int devfs_open(void * opaque, const char * path, int flags, int mode) {
    uint32_t h = hash_djb2((const uint8_t *) path, -1);
//    DBGOUT("devfs_open(%p, \"%s\", %i, %i)\r\n", opaque, path, flags, mode);
    switch (h) {
    case stdin_hash:
        if (flags & (O_WRONLY | O_RDWR))
            return -1;
        return fio_open(stdin_read, NULL, NULL, NULL, NULL);
        break;
    case stdout_hash:
        if (flags & O_RDONLY)
            return -1;
        return fio_open(NULL, stdout_write, NULL, NULL, NULL);
        break;
    case stderr_hash:
        if (flags & O_RDONLY)
            return -1;
        return fio_open(NULL, stdout_write, NULL, NULL, NULL);
        break;
    }
    return -1;
}

static int devfs_open_dir(void * opaque, const char * path){
    if( strlen(path) == 0 ){
        // TODO : Add function
        return dir_open(NULL,NULL,NULL); 
    }else{
        return OPENDIR_NOTFOUND;
    }
}

void register_devfs() {
    DBGOUT("Registering devfs.\r\n");
    register_fs("dev", devfs_open, devfs_open_dir, NULL);
}
