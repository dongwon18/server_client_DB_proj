---
finished_date: 2022-04-13
tags:
    - server
    - client
    - thread
    - shell_programming
    - Ubuntu
    - C
    - vi
    - Makefile
---
# server client DB project
## Environment
- Ubuntu 20.04 LTS
- GCC
- UTF-8
## server
- get message from clients
- save, read, erase variables according to the client's command as DB server
- DB is implemented as struct array
    ```c
    typedef struct _NODE{
        char name[NAME_SIZE];
        char value[BUF_SIZE];
    }NODE;

    NODE variables[100];
    ```
- using mutex, create and close socket for the client
- using thread, get client's message and treat client's command
- detailed implemenatation information is included in src code as comment

### Notice
- maximum 256 clients can access server at the same time
- maximum 100 variables can be saved in the server
- maximum message size: 99 bytes (except '\0')
- maximum client name size: 19 bytes (except '\0')
- port number

### supported command
#### command from client
- save
    - save *variable name*:*variable value*
    - save variable with name and value in variables array if the variable has not declared yet
    - modify variable's value if the variable already exists
    - print the result in prompt
    
- read
    - read *variable name*
    - read the variable's value 
    - send the value to client if the variable exists
    - if the variable does not exist, send *No such variable* message

- clear
    - clear
    - erase all variables in variable array
    - set *vari_cnt* to 0
### command from keyboard
- ctrl + c
    - handle *SIGINT* signal with *INThandler*
    - terminate the thread
    - close sockets of clients
    - terminate the program

## client
- get keyboard input from user
- send message to server and get result
- getting keyboard input and recieving message from server are done simultaneously with thread

### Notice
- maximum message size: 99 bytes (except '\0')
- maximum client name size: 19 bytes (except '\0')
- set user's name as *[DEFAULT]*
- set port number as *12345*

### supported command
#### command from client
- connect
    - connect *ip address*
    - connect to the server whose ip address is *ip address*

- save
    - save *variable name*:*variable value*
    - send request to the server
    
- read
    - read *variable name*
    - send request to the server
    - recieve the result from the server

- clear
    - clear
    - send request to the server

- exit
    - exit
    - send *SIGINT* signal to terminate the program

### command from keyboard
- ctrl + c
    - handle *SIGINT* signal with *INThandler*
    - terminate the thread
    - close a socket
    - terminate the program

## result
- client result

<p align=center>
    <img src="./result/client result.png" alt="prompt running client"><br/>
    client result in prompt
</p>

- server result

<p align=center>
    <img src="./result/server result.png" alt="prompt running server"><br/>
    server result in prompt
</p>

## How to run
1. make file
    ```
    make clear
    make shell_server
    make shell_client
    ```
2. run server
    ```
    ./server_shell [port no.]
    ex) ./server_shell 12345
    ```
3. run client
    ```
    ./client_shell
    ```
## File structure
```
|-- src
    |-- Makefile
    |-- client_shell.c
    |-- server_shell.c
|-- result
    |-- client result.png
    |-- server result.png
```
## 배운 점
- framework를 이용한 socket programming에 익숙해졌다.
- thread를 이용하여 동시에 사용자에게 입력을 받으면서 서버로부터 메시지를 받을 수 있다.
- 여러 명의 client가 동시에 접근하는 server를 구현할 수 있다.
- ctrl + c를 입력하여 프로그램을 종료할 때도 안전하게 종료할 수 있다.
## 한계점
- client에서 서버로부터 받은 메시지를 깔끔하게 출력하기 힘들다. 메시지를 출력 후 command의 제목이 나오지 않는 버그가 존재한다. 그러나 이 상태에서 command를 입력하면 올바르게 동작한다.
- 잘못된 command 형식을 입력하였을 때 경고와 함께 재입력을 요구하는 부분을 구현하지 않았다.
- help를 제공하지 않았다.
