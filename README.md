# Socket-based-FTP	

The following is an implementation of File Transfer Protocol using TCP sockets in C. All 4 functionalities have been implemented in the the program which are

 * PUT : Transfer a file from client to server
 * GET : Transfer a file from server to client
 * MGET : Transfer all files of a given extension from server to client
 * MPUT : Transfer all files of a given extension from client to server

File overwrite feature has been implemented along with it i.e. if the destination has a file with the same
name of the file to be transferred, the client will be asked whether to overwrite the file or not.

### About the code

The source code for server and client is given in "server.c" and "client.c".

The folders ‘dsk_client’ and ‘dsk_server’ act as the client’s and server’s disk. All the files are transferred within them. Each program, client and the server, reads their respective folder to keep a record of the files in their disk.


For compiling the code, 

On server 
```bash
$ gcc server.c -o server
```

On client
```bash
$ gcc client.c -o client
```

To run the code, 
First on the server node
```bash
$ ./server <Server Port number>
```
The on Client’s terminal run
```bash
$ ./client <Server IP Address> <Server Port number>
```

After running the code, enter the commands in client’s terminal in legal command format. The server code needs to executed once and shall be closed using “Ctrl + C” in the server machine. Every client on execution develops a connection with the server and the same socket connection is maintained. until the client program exit. USE COMMAND “EXIT” to quit client program. This will disconnect the
client. Running the client program again shall develop a new connection again.

### Commands
   * PUT -> PUT \<filename>
   * GET -> GET \<filename>
   * MGET ->  MGET \<extension including dot> 
    	( For example : MGET .c)
   * MPUT -> MPUT \<extension including dot>
		 ( For example : MPUT .c )
         
         
### Assumptions
   * The user shall give the input in legal format. Incomplete command or illegal characters in the command might lead to runtime crash and has not been handled assuming user shall provide the command in legal format.
   * Filename must be a string of ASCII characters without space and having length less than MAX_LENGTH ( default is 512 bytes), a macros mentioned in the files ‘server.h’ and ‘client.h’. To send file with larger filenames, you must modify the macros and recompile.
   * The folder ‘dsk_client’ and ‘dsk_server’ are taken to be the disk of server and client node. This is to ensure that the program can be evaluated and tested on any machine easily without worrying about addressing problem.