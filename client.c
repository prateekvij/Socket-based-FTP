/*
The following the client-side implementation of socket-based File Transfer Protocol in C
The implementation has been done for linux system

Run the compiled code on the server node via
$ ./client <Server IP Address> <Server Port number>

To recompile the code, 
$ gcc client.c -o client

The following set of functionalities has are supported
PUT : Transfer a file from client to server
GET : Transfer a file from server to client
MGET : Transfer all files of a given extension from server to client
MPUT : Transfer all files of a given extension from client to server

Command Format
PUT​ -> ​ PUT <filename>
GET​ -> ​ GET <filename>
MGET​ -> ​ MGET <extension including dot>
	For example : MGET .c
MPUT​ -> ​ MPUT <extension including dot>
	For example : MPUT .c

*/

#include "client.h"

int main(int argc, char const *argv[])
{
	// Input validation
	if (argc != 3)
	{
		printf("Invalid execution format. Use the following format\n<executable code><Server IP Address><Server Port number>\n");
		exit(-1);
	}

	// basic declaraion
	struct sockaddr_in address;
	int client_sock;
	struct sockaddr_in server_addr;
	char buffer[MAX_LENGTH];
	bzero(buffer, MAX_LENGTH);

	// socket creation
	int PORT = atoi(argv[2]);
	const char* IP = argv[1];
	if (!(client_sock = socket(AF_INET, SOCK_STREAM, 0)))
		error("Failed to create a socket. Exiting! ");

	// server address configuration
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if(inet_pton(AF_INET, IP, &server_addr.sin_addr)<=0) 
		error("\nError: Invalid address. Address not supported. ");

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		error("\nError: Failed to connect to the remote host. ");

	while(1){
		bzero(buffer, MAX_LENGTH);
		fgets(buffer, MAX_LENGTH, stdin);
		buffer[strcspn(buffer, "\n")] = 0;

		// decide for the correct operation from put/get/mget/mput
		char cmd_line[MAX_LENGTH];
		strcpy(cmd_line,buffer);
		char* cmd = strtok(cmd_line," ");

		// If operation is put
		if (strcmp("PUT",cmd)==0)
		{
			// check if file exist
			char *fname;
			cmd =  strtok(NULL," ");
			fname = cmd;
			if (!fileExist(fname)){
				printf("ERROR: File %s not found.\n", fname);
				continue;
			}

			// send command to server
			printf("CLIENT : transferring %s to server.\n",buffer );
			send(client_sock, buffer, MAX_LENGTH, 0);
			put_file(client_sock, fname);
			bzero(buffer, MAX_LENGTH);
            recv(client_sock, buffer, MAX_LENGTH, 0);
            if (strcmp(buffer,"SUCCESS")){
            	printf("Error sending file. Please try again\n");
            	break;
            }

			
		}
		else if (strcmp("GET",cmd)==0)
		{
			// check if file exist
			char *fname;
			cmd =  strtok(NULL," ");
			fname = cmd;

			// send command to server
			send(client_sock, buffer, MAX_LENGTH, 0);

			// Check for server reponse
			get_file(client_sock, fname);
			
		}
		else if (strcmp("MPUT",cmd)==0)
		{
			// send(client_sock, buffer, MAX_LENGTH, 0);
			
			char *fext, *fname;
			cmd =  strtok(NULL," ");
			fext = cmd;
			printf("Sending all %s files\n\n",fext );
			DIR *di;
			struct dirent *dir;
			di = opendir(DSK);
			while ((dir = readdir(di)) != NULL){
				fname= dir->d_name;
				char* ext = strrchr(fname, '.');
				if (strcmp(ext,fext)==0)
				{
					bzero(buffer,MAX_LENGTH);
					strcpy(buffer,"PUT ");
					strcat(buffer,fname);
					printf("Sending %s to server\n",buffer );
					send(client_sock, buffer, MAX_LENGTH, 0);
					put_file(client_sock,fname);
					bzero(buffer, MAX_LENGTH);
                    recv(client_sock, buffer, MAX_LENGTH, 0);
                    if (strcmp(buffer,"SUCCESS"))
                    {
                    	printf("Some error occurred\n");
                    	break;
                    }


				}
			}
			closedir(di);
		}

		else if (strcmp("MGET",cmd)==0)
		{
			char *fext, *fname;
			cmd =  strtok(NULL," ");
			fext = cmd;
			printf("Getting all %s files\n",fext );

			// send command to server
			
			send(client_sock, buffer, MAX_LENGTH, 0);
			while(1){
				bzero(buffer, MAX_LENGTH);
				recv(client_sock, buffer, MAX_LENGTH, 0);
				if (strcmp(buffer,"OVER")!=0)
				{
					char fname[MAX_LENGTH];
					strcpy(fname, buffer);
					// printf("%s\n",buffer );

					if (fileExist(fname))
					{
						printf("Do you want to overwrite the file %s (yes/no) ? ",fname);
						bzero(buffer, MAX_LENGTH);
						fgets(buffer, MAX_LENGTH, stdin);
						buffer[strcspn(buffer, "\n")] = 0;
						if (strcmp(buffer,"yes")==0)
						{
							bzero(buffer, MAX_LENGTH);
							strcpy(buffer, "SEND");
							send(client_sock, buffer, MAX_LENGTH, 0);

							bzero(buffer, MAX_LENGTH);
							printf("Receiving file content for %s\n",fname );
							receive_file(client_sock,fname);

							bzero(buffer, MAX_LENGTH);
	                        strcpy(buffer, "READY");
	                        send(client_sock, buffer, MAX_LENGTH, 0);
						}
						else{
							bzero(buffer, MAX_LENGTH);
							strcpy(buffer, "SKIP");
							send(client_sock, buffer, MAX_LENGTH, 0);
						}
					}
					else{
						bzero(buffer, MAX_LENGTH);
						strcpy(buffer, "SEND");
						send(client_sock, buffer, MAX_LENGTH, 0);

						bzero(buffer, MAX_LENGTH);
						printf("Receiving file content for %s\n",fname );
						receive_file(client_sock,fname);

						bzero(buffer, MAX_LENGTH);
                        strcpy(buffer, "READY");
                        send(client_sock, buffer, MAX_LENGTH, 0);
					}

				}
				else{
					printf("All files received\n");
					bzero(buffer, MAX_LENGTH);
					strcpy(buffer, "DONE");
					send(client_sock, buffer, MAX_LENGTH, 0);
					break;
				}
			}
		}
		
		else if (strcmp("EXIT",buffer)==0){
			send(client_sock, buffer, MAX_LENGTH, 0);
			break;
		}
		else
		{
			printf("Invalid Command\n");
			continue;
		}
		printf("File Transfer Protocol successful!!\n\n");
		
	}
	close(client_sock);
	return 0;
}


int fileExist(char *fname){
	int found = 0;
	DIR *di;
	struct dirent *dir;
	di = opendir(DSK);
	while ((dir = readdir(di)) != NULL){
		if(strcmp(dir->d_name,fname)==0){
			found=1;
			break;
		}
	}
	closedir(di);
	return found;
}


int send_file(int socket, char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *file = fopen(fpath, "r");
    if(file == NULL)
    {
        printf("ERROR: File %s not found.\n", fname);
        return -1;
    }

    bzero(buffer, MAX_LENGTH); 
    int fs_block_sz; 
    while((fs_block_sz = fread(buffer, sizeof(char), MAX_LENGTH, file)) > 0){
        if(send(socket, buffer, fs_block_sz, 0) < 0){
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fname, errno);
            break;
        }
        bzero(buffer, MAX_LENGTH);
    }
    return 0;
}

int receive_file(int socket,char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *out_file = fopen(fpath, "wb");
    if(out_file == NULL)
        printf("File %s Cannot create file on your pc.\n", fname);
    else{
        bzero(buffer, MAX_LENGTH); 
        int out_file_block_sz = 0;
        while((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0) {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if(write_sz < out_file_block_sz)
            {
                error("File write failed on your pc.");
            }
            
            bzero(buffer, MAX_LENGTH);
            if (out_file_block_sz == 0 || out_file_block_sz != MAX_LENGTH ) 
            {
                break;
            }
        }
        if(out_file_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                printf("recv() timed out.\n");
            }
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                exit(1);
            }
        }
        printf("Received the file from Server's Disk!\n\n");
        fclose(out_file); 
    }

}

void put_file(int client_sock,char* fname){
	// Check for server reponse
	char buffer[MAX_LENGTH];
	bzero(buffer,MAX_LENGTH);
	recv(client_sock, buffer, MAX_LENGTH, 0);

	if (strcmp(buffer,"ABORT")==0)
	{
		printf("Operation aborted1.\n");
	}

	if (strcmp(buffer,"OKAY")==0)
	{
		printf("File not present in Server's Disk! \n\n");
		send_file(client_sock, fname);
		
	}
	else{
		printf("File present in Server's disk! \nDo you wish to overwrite the File %s ? (yes/no)\n",fname);
		bzero(buffer, MAX_LENGTH);
		fgets(buffer, MAX_LENGTH, stdin);
		buffer[strcspn(buffer, "\n")] = 0;
		send(client_sock, buffer, MAX_LENGTH, 0);
		if (strcmp(buffer,"yes")==0)
		{
			bzero(buffer,MAX_LENGTH);
			recv(client_sock, buffer, MAX_LENGTH, 0);
			// printf("CLIENT response OKAy %s\n",buffer );
			if (strcmp(buffer,"OKAY")==0)
			{
				send_file(client_sock, fname);
				bzero(buffer, MAX_LENGTH);
			}
			else{
				printf("Operation aborted.\n");
			}
			printf("File Transfer Protocol Completed! \n\n");
		}
		else{
			recv(client_sock, buffer, MAX_LENGTH, 0);
			if (strcmp("ABORT",buffer))
			{
				printf("Unexpected response from server\n");
			}
			printf("Operation aborted.\n\n");
		}
	}
}
void get_file(int client_sock, char* fname){
	char buffer[MAX_LENGTH];
	bzero(buffer,MAX_LENGTH);
	recv(client_sock, buffer, MAX_LENGTH, 0);

	if (strcmp(buffer,"ABORT")==0)
	{
		printf("Operation aborted1.\n");
	}
	else if (strcmp(buffer,"READY")==0)
	{
		if (fileExist(fname))
		{
			printf("Do you want to overwrite the file (yes/no) ? ");
			bzero(buffer, MAX_LENGTH);
			fgets(buffer, MAX_LENGTH, stdin);
			buffer[strcspn(buffer, "\n")] = 0;
			send(client_sock, buffer, MAX_LENGTH, 0);
			if (strcmp(buffer,"yes")==0)
			{
				
				receive_file(client_sock, fname);
				// printf("FIle can be received\n");

			}
			else{
				printf("Sending no to server\n");
				recv(client_sock, buffer, MAX_LENGTH, 0);
				if (strcmp("ABORT",buffer))
				{
					printf("Unexpected response from server\n");
				}
				printf("Operation aborted2.\n");
			}
		}
		else{
			bzero(buffer,MAX_LENGTH);
			strcpy(buffer,"yes");
			send(client_sock, buffer, MAX_LENGTH, 0);
			bzero(buffer, MAX_LENGTH);
			receive_file(client_sock, fname);
			bzero(buffer, MAX_LENGTH);
		}
	}
	else{
		printf("File %s not found.\n",fname);//Operation Aborted\n", fname);
	}
}