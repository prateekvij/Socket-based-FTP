#include "server.h"

int main(int argc, char const *argv[])
{
	// Input Validation
	if (argc != 2){
		printf("Invalid execution format. Use the following format\n<executable code> <Server Port number>\n");
		exit(-1);
	}

	// Basic Declarations for socket programming
	int PORT;
	int server_sock;
	struct sockaddr_in client_addr;
	char buffer[MAX_LENGTH];
	bzero(buffer, MAX_LENGTH);

	// Create a socket for communication
	PORT = atoi(argv[1]);
	if (!(server_sock = socket(AF_INET, SOCK_STREAM, 0)))
		error("Failed to create a socket. Exiting!");

	// configure client address details
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(PORT);

	// Bind the socket to the application
	if (bind(server_sock,(struct sockaddr*)&client_addr,(socklen_t)sizeof(client_addr)) < 0)
		error("Binding failed. Exiting!");

	// Start listening to the port
	if (listen(server_sock, MAX_CONN)<0)
		error("Unable to listen to the port");

	// printf("fo\n");
	
	// Create a loop for accepting connections
	while(1){
		int accept_sockfd;
		int addr_len = sizeof(client_addr);
        printf("Awaiting Connection..\n");

		if( (accept_sockfd = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len)) < 0)
			error("accept");
        printf("Connection received\n");
		
		while(1){
            
			// receive command from user
			recv(accept_sockfd, buffer, MAX_LENGTH, 0);
			// printf("%s",buffer );
			// if (buffer)
			// {
			// 	break;
			// }
			char cmd_line[MAX_LENGTH];
			strcpy(cmd_line,buffer);
			char* cmd = strtok(cmd_line," ");
			printf("%s\n",cmd );
			if (strcmp(cmd,"EXIT")==0){
				printf("Exiting\n");
				break;
			}
			else if (strcmp(cmd,"GET")==0)
			{

                char *fname;
                cmd =  strtok(NULL," ");
                fname = cmd;
                get_file(accept_sockfd, fname);
                
  			}
			else if (strcmp(cmd,"PUT")==0)
			{
				char *fname;
				cmd =  strtok(NULL," ");
				fname = cmd;
                put_file(accept_sockfd, fname);
                bzero(buffer,MAX_LENGTH);
                strcpy(buffer,"SUCCESS");
                send(accept_sockfd, buffer, MAX_LENGTH, 0);
				
			}
			else if (strcmp(cmd,"MGET")==0)
			{
                char *fext, *fname;
                cmd =  strtok(NULL," ");
                fext = cmd;
                printf("Sending all %s files\n",fext );
				DIR *di;
                struct dirent *dir;
                di = opendir(DSK);
                bzero(buffer,MAX_LENGTH);
                while ((dir = readdir(di)) != NULL){
                    fname= dir->d_name;
                    // printf("--file : %s\n", fname);
                    char* ext = strrchr(fname, '.');
                    if (ext==NULL)
                    {
                        continue;
                    }
                    if (strcmp(ext,fext)==0)
                    {
                        
                        bzero(buffer,MAX_LENGTH);
                        strcpy(buffer,fname);
                        printf("%s to be sent to client\n",buffer );
                        send(accept_sockfd, buffer, MAX_LENGTH, 0);
                        // printf("b %s\n",buffer);
                        bzero(buffer, MAX_LENGTH);
                        
                        recv(accept_sockfd, buffer, MAX_LENGTH, 0);
                        printf("%s\n",buffer );
                        if (strcmp(buffer,"SKIP")==0)
                        {
                            continue;
                        }
                        else if (strcmp(buffer,"SEND")==0)
                        {
                            bzero(buffer,MAX_LENGTH);
                            printf("Sending file %s to Client's Disk\n\n",fname );
                            send_file(accept_sockfd, fname);
                        }

                        bzero(buffer, MAX_LENGTH);
                        recv(accept_sockfd, buffer, MAX_LENGTH, 0);
                        if (strcmp(buffer,"READY"))
                        {
                            printf("Some error occured\n");
                            break;
                        }
  
                    }
                }
                
                bzero(buffer,MAX_LENGTH);
                strcpy(buffer,"OVER");
                send(accept_sockfd, buffer, MAX_LENGTH, 0);
                bzero(buffer, MAX_LENGTH);
                recv(accept_sockfd, buffer, MAX_LENGTH, 0);
                printf("All files of %s sent\n",fext );

                closedir(di);
			}
			else{
				printf("\nInvalid\n");
			}
			
			bzero(buffer, MAX_LENGTH);
		}
				

    	close(accept_sockfd);
        printf("Connection closed\n");
		
	}
	close(server_sock);


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

int receive_file(int socket,char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *out_file = fopen(fpath, "wb");
    if(out_file == NULL)
        printf("File %s Cannot create file on server.\n", fname);
    else
    {
        bzero(buffer, MAX_LENGTH); 
        int out_file_block_sz = 0;
        while((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0) 
        {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if(write_sz < out_file_block_sz)
            {
                error("File write failed on server.");
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
        printf("Received file from Client!\n\n");
        fclose(out_file); 
    }

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
    while((fs_block_sz = fread(buffer, sizeof(char), MAX_LENGTH, file)) > 0)
    {
        if(send(socket, buffer, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fname, errno);
            break;
        }
        bzero(buffer, MAX_LENGTH);
    }
    return 0;

}

void put_file(int accept_sockfd,char* fname){
    char buffer[MAX_LENGTH];
    if(fileExist(fname)){
        printf("File Exist\n");
        printf("Do you wish to overwrite the File Contents?\n");
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer,"CONTINUE");
        send(accept_sockfd,buffer,MAX_LENGTH,0);

        bzero(buffer, MAX_LENGTH);
        recv(accept_sockfd, buffer, MAX_LENGTH,0);
        printf("Response : %s\n",buffer );
        if(strcmp(buffer,"yes")==0){
            bzero(buffer, MAX_LENGTH);
            strcpy(buffer,"OKAY");
            send(accept_sockfd,buffer,MAX_LENGTH,0);
            receive_file(accept_sockfd,fname);
            printf("File sucessfully received\n\n");
        }
        else{
            bzero(buffer,MAX_LENGTH);
            strcpy(buffer,"ABORT");
            send(accept_sockfd,buffer,MAX_LENGTH,0);
            printf("Aborted\n\n");
        }
    }
    else{
        printf("File not present in the server's disk.\nReady to receive the file!\n\n");
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer,"OKAY");
        send(accept_sockfd,buffer,MAX_LENGTH,0);
        receive_file(accept_sockfd,fname);
    }
}

void get_file(int accept_sockfd, char* fname){
    char buffer[MAX_LENGTH];
    if(fileExist(fname)){
        printf("File Exist on the server's disk\n");
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer,"READY");
        send(accept_sockfd,buffer,MAX_LENGTH,0);

        recv(accept_sockfd, buffer, MAX_LENGTH,0);
        // printf("Response on ready : %s\n",buffer );
        if(strcmp(buffer,"yes")==0){
        	// printf("Response : yes\n");
            send_file(accept_sockfd, fname);
        }
        else{
            printf("Response : no\n");
            bzero(buffer,MAX_LENGTH);
            strcpy(buffer,"ABORT");
            send(accept_sockfd,buffer,MAX_LENGTH,0);
            printf("File transfer aborted\n");
        }
        printf("File Transfer protocol Completed!\n\n");
    }
    else{
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer,"CANCEL");
        send(accept_sockfd,buffer,MAX_LENGTH,0);
    }
}