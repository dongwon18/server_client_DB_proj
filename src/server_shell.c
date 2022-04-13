/*
 * Copyright 2022. Dongwon Kim All rights reserved.
 *
 * File name: server_shell.c
 *
 * Written by Dongwon Kim
 *
 * server for DB shell project
 * 	server that save and read variables
 * 	
 * Modification History
 * 	written by Dongwon Kim on Aprin 13, 2022
 *
 * Environment
 * 	Ubuntu 20.04 LTS
 * 	GCC
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define MAX_CLNT 256
#define MAX_VARI 100  // max 100 variables can be saved

// struct for saving variable
typedef struct _NODE{
	char name[NAME_SIZE];
	char value[BUF_SIZE];
}NODE;

// function declare
void *handle_clnt(void *arg);
int find_name(char *name);
void handle_msg(char *msg, int len, int clnt_sock);
void error_handling(char *msg);
void INThandler(int sig_no);

// global variables
int clnt_cnt = 0;          //  no. of client(max 256)
int vari_cnt = 0;          //  no. of variables(max 100)
int clnt_socks[MAX_CLNT];  // array to save socket no. of clients
pthread_mutex_t mlock;
pthread_t tid;
NODE variables[MAX_VARI];  //  to save variables

/*
 * connect to clients
 * port number should be 12345
 */
int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;

	int option;
	socklen_t optlen;

	if(argc != 2){
		printf("[USAGE] %s <port>\n", argv[0]);
		exit(1);
	}
	
	// safe terminate
	signal(SIGINT, INThandler);

	pthread_mutex_init(&mlock, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	optlen = sizeof(option);
	option = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr))==-1){
		error_handling("bind() error");
	}

	if(listen(serv_sock, 5)==-1){
		error_handling("listen() error");
	}
	while(1){
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mlock);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mlock);

		pthread_create(&tid, NULL, handle_clnt, (void *)&clnt_sock);
		pthread_detach(tid);
		printf("Conntected client IP: %s\n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

/*
 * read mssg from client(whose socket no == clnt_sock)
 * until client terminates
 * 
 * if the client terminates
 *     close the socket and modify clnt_socks arr
 * 
 * input
 * 	void *arg: socket number
 * 
 * output
 * 	modify clnt_socks arr
 * 	modify variables arr according to the mssg
 */
void *handle_clnt(void *arg){
	int clnt_sock = *((int *)arg);
	int str_len = 0;
	char msg[BUF_SIZE];

	// until the client terminates handle mssg
	while((str_len = read(clnt_sock, msg, sizeof(msg)))!=0){
		handle_msg(msg, str_len, clnt_sock);
	}
	printf("Client %d leaved\n", clnt_sock);
	
	// refhresh clnt_socks arr
	pthread_mutex_lock(&mlock);

	for(int i = 0; i < clnt_cnt; i++){
		if(clnt_sock == clnt_socks[i]){
			while(i++<clnt_cnt -1){
				clnt_socks[i] = clnt_socks[i+1];
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mlock);

	// close the socket
	close(clnt_sock);
	return NULL;
}

/*
 * find if there is a variable with the given name
 * (linear search)
 * 
 * input
 * 	char *name: name of variable
 * 
 * output
 * 	return int -1: if there is no variable with such name
 * 	return int: if there is the variable, return index of the variable in variables[]
 */
int find_name(char *name){
	for(int i = 0; i < vari_cnt;i++){
		if(strcmp(variables[i].name, name)==0){
			return i;
		}
	}
	return -1;
}

/*
 * tokenize client's message
 * do save, read, clear according to the message
 * 
 * input
 * 	char *msg: message from client
 * 		[user name] cmd 
 * 	int len: length of the message
 * 	int clnt_sock: socket number of the client
 * 
 * output:
 * 	modify variables[]
 */
void handle_msg(char *msg, int len, int clnt_sock){
	// the other clients are not allowed to touch variables[]
	pthread_mutex_lock(&mlock);
	msg[len-1] = 0;  // mark last

	// tokenize
	char *cli_name = strtok(msg, " ");
	char *cmd = strtok(NULL, " ");

	/*
	 * save [name]:[value]
	 * store name, value as string in variables if there is no such name
	 * if the name already exists, modify the value to new value.
	 */
	if(strcmp(cmd, "save")==0){
		char *name = strtok(NULL, ":");
		char *value = strtok(NULL, ":");
		
		int index = find_name(name);
		//  no such name
		if(index == -1){
			strcpy(variables[vari_cnt].name, name);
			strcpy(variables[vari_cnt].value, value);
			printf("%s %s saved\n", variables[vari_cnt].name,variables[vari_cnt].value );
			vari_cnt++;
		//  name exists
		}else{
			strcpy(variables[index].value, value);
			printf("%s %s modified\n", variables[index].name, variables[index].value);
		}
	
	/*
	 * read [name]
	 *
	 * read value of the name and send it to the client
	 * if there is no such name, send 'NO such variable' mssg
	 */
	}else if(strcmp(cmd, "read")==0){
		char *name = strtok(NULL, " ");
		int index = find_name(name);

		//  no such name
		if(index == -1){
			char message[BUF_SIZE] = "NO such variable";
			write(clnt_sock, message, strlen(message));
		//  name exists
		}else{
			write(clnt_sock, variables[index].value, strlen(variables[index].value));
		}
	
	/*
	 * clear
	 *
	 * remove all stored variables
	 * set all variables's value to ""
	 * set vari_cnt to 0
	 */
	}else if(strcmp(cmd, "clear")==0){
		for(int i = 0; i < vari_cnt; i++){
			strcpy(variables[i].name, "");
			strcpy(variables[i].value, "");
		}
		vari_cnt = 0;
	}
	pthread_mutex_unlock(&mlock);
}


void error_handling(char *msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
 * if ctrl + C is entered,
 * terminate program safely
 */
void INThandler(int sig_no){
	for(int i = 0; i < clnt_cnt; i++){
		close(clnt_socks[i]);
	}
	pthread_cancel(tid);
	pthread_join(tid, NULL);
	printf("Terminating Server\n");
	exit(0);
}

