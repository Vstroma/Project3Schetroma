#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_PORT_NUMBER 8888
#define MAX_BUFFER_SIZE 4000

struct Connection {     
    int socket_desc;
    time_t timestamp;
    int priority;
};

struct Connection connection_queue[MAX_BUFFER_SIZE];
int connection_front = 0;
int connection_rear = -1;
int connection_count = 0;

struct Log {
    char message[256];
};

struct Log log_queue[MAX_BUFFER_SIZE];
int log_front = 0;
int log_rear = -1;
int log_count = 0;

pthread_mutex_t connection_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t connection_full = PTHREAD_COND_INITIALIZER;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log_full = PTHREAD_COND_INITIALIZER;

void *threadWorker(void *arg);
void *threadLogger(void *arg);
void handleSpellCheck(int socketClient);




int main() {
    int port_number = DEFAULT_PORT_NUMBER;      //assgian port number
    int socket_descriptor, new_socket, c;
    struct sockaddr_in server, client;


    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);    //create the server socket
    if (socket_descriptor == -1) {
        puts("Error could not create socket.");
        exit(1);
    }

    server.sin_family = AF_INET;            //set up server address structure
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);


    //bing the socket to the server address
    int bind_result = bind(socket_descriptor, (struct sockaddr *)&server, sizeof(server));
    if (bind_result < 0) {
        puts("Error could not bind socket.");
        exit(1);
    }
    puts("Successfully bound to port ___.");

   
    listen(socket_descriptor, 3);       // use listen for incoming connections
    puts("Waiting for incoming connections");

    //accepting incoming connections
    while (1) {
        c = sizeof(struct sockaddr_in);

        //accept a new connection
        new_socket = accept(socket_descriptor, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket < 0) {
            puts("Error connection failed.");
            continue;           // contine for new connections
        }
        puts("Connection successful.");

        pthread_mutex_lock(&connection_mutex);      // lock the connection queue mutex before accessing it

        // add new connection details to the connection queue
        connection_queue[++connection_rear].socket_desc = new_socket;
        connection_queue[++connection_rear].timestamp = time(NULL);
        connection_queue[++connection_rear].priority = 0;
        connection_count++;
        pthread_cond_signal(&connection_full);      // signal queue is not full
        pthread_mutex_unlock(&connection_mutex);        //unlcok the connection queue mutex
    }

    //signal if there are connections in the queue
    if (connection_count > 0) {
        pthread_cond_signal(&connection_full);
    }
}

void *threadWorker(void *arg) {

    intptr_t socket_descriptor = (intptr_t)arg; //casting socket descriptor tp intptr_t

    int new_socket, c;
    struct sockaddr_in server, client;

    new_socket = socket(AF_INET, SOCK_STREAM, 0);   //creating new socket
    if (new_socket < 0) {
        puts("Error could not create socket.");
        exit(1);
    }

    server.sin_family = AF_INET;        //setting up server address structure
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT_NUMBER);

    int bind_result = bind(new_socket, (struct sockaddr *)&server, sizeof(server)); //bind to bind socket to server address
    if (bind_result < 0) {                                              
        puts("Error could not bind socket.");
        exit(1);
    }
    return NULL;        //exit thread
}

void *threadLogger(void *arg) {

    intptr_t socket_descriptor = (intptr_t)arg;  // cast socket descriptor to intptr_t

    int new_socket, c;
    struct sockaddr_in server, client;

    new_socket = socket(AF_INET, SOCK_STREAM, 0);   //create a new socket for logging
    if (new_socket < 0) {
        puts("Error could not create socket.");
        exit(1);
    }

    server.sin_family = AF_INET;        //set up server adress structure 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT_NUMBER);

    //bind the logging socket to the server address
    int bind_result = bind(new_socket, (struct sockaddr *)&server, sizeof(server));     
    if (bind_result < 0) {
        puts("Error could not bind socket.");
        exit(1);
    }

    return NULL;        // Return null to exit thread
}
