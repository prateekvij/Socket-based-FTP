#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h> 

#define MAX_LENGTH 512
#define MAX_CONN 5
#define DSK "./dsk_server/"

void put_file(int socket,char* fname);
void get_file(int socket, char* fname);

int receive_file(int socket,char* fname);
int send_file(int socket, char* fname);

int fileExist(char *fname);
int error(char *err){
	perror(err);
	exit(EXIT_FAILURE);
}