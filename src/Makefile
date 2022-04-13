CC=gcc
CFLAGS= -D_REENTRANT -lpthread

shell_client: client_shell.c
	$(CC) client_shell.c -o ./client_shell $(CFLAGS)

shell_server: server_shell.c
	$(CC) server_shell.c -o ./server_shell $(CFLAGS)

clean:
	rm ./client_shell ./server_shell
