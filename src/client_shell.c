/*
 * Copyright 2022. Dongwon Kim All rights reserved.
 *
 * File name: client_shell.c
 *
 * Written by Dongwon Kim
 *
 * client for DB shell project
 * 	client shell that connect to the server, save, read, clear variables
 * 
 * supported command
 * 	connect [ip]: connect to the server whose ip addr is ip with port no 12345
 * 	save [name]:[value]: save a variable whose name is [name] and value is [value] to server
 * 	read [name]: get variable's value from server
 * 	clear: erase all stored variables in server
 * 	exit: terminate the program by sending SIGINT signal
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
#include <pthread.h>
#include <signal.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

// function declare
void *client_shell(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
void INThandler(int sig);
void myconnect(char *ip, char *port);

// global variables
char name[NAME_SIZE]  = "[DEFAULT]";  // user name
char msg[BUF_SIZE];
int sock;
pthread_t shell_thread, rcv_thread;
struct sockaddr_in serv_addr;

int main(int argc, char *argv[]){
	// for safe terminate
	signal(SIGINT, INThandler);
	
	void *thread_return;
	
	pthread_create(&shell_thread, NULL, client_shell, NULL);
	pthread_join(shell_thread, &thread_return);

	pthread_join(rcv_thread, &thread_return);

	close(sock);
	return 0;
}

/*
 * get user input
 *
 * input : none
 * 
 * output: 
 * 	modify sock, serv_addr
 */
void *client_shell(void *arg){
	char user_input[BUF_SIZE];
	while(1){
		printf("client shell >> ");
		fgets(user_input, BUF_SIZE, stdin);

		// backup of user_input for strtok
		char temp_input[BUF_SIZE];
		strcpy(temp_input, user_input);

		//tokenize
		char *cmd = strtok(temp_input, " ");

		/*
		 * connect to given ip
		 * create thread for recieving
		 */
		if(strcmp(cmd, "connect")==0){
			char *ip = strtok(NULL, " ");
			char *port = "12345";
			myconnect(ip, port);
			pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
		
		/*
		 * terminate the program
		 */
		}else if(strcmp(user_input, "exit\n") == 0){
				kill(0, SIGINT);
				return NULL;
		
		/*
		 * send user input to server
		 * save, read, clear cmd can be sent
		 */
		}else{
			char name_msg[NAME_SIZE + BUF_SIZE];
			sprintf(name_msg, "%s %s", name, user_input);
			write(sock, name_msg, strlen(name_msg));
		}		
	}
	return NULL;
}

/*
 * connect to the server
 *
 * input:
 * 	char *ip: ip addr from user input
 * 	char *port: port no, default 12345
 * 
 * output:
 * 	modify sock, serv_addr
 */
void myconnect(char *ip, char *port){
	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(atoi(port));

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}
}

/*
 * recieve mssg from server and print it to shell
 * run as thread
 * 
 * input
 * 	void *arg: socket number
 * 
 * output
 * 	print recieved mssg to shell
 */
void *recv_msg(void *arg){
	int sock = *((int *)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;
	while(1){
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
		if(str_len == -1||str_len == 0){
			kill(0, SIGINT);

			return NULL;
		}
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
		fputc('\n',stdout);
	}
	return NULL;
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
void INThandler(int sig){
	pthread_cancel(shell_thread);
	pthread_cancel(rcv_thread);

	pthread_join(shell_thread, NULL);
	pthread_join(rcv_thread, NULL);

	close(sock);
	printf("Terminating Client\n");
	exit(1);
}
