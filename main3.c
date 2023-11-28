#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_PORT_NUMBER 8888
#define MAX_BUFFER_SIZE 4096

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


int main() {
    int port_number = DEFAULT_PORT_NUMBER;      //assign port number
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

   
    pthread_t workerThread;     // creates worker thread
    pthread_create(&workerThread, NULL, threadWorker, NULL);

    pthread_t loggerThread;     //creates logger thread
    pthread_create(&loggerThread, NULL, threadLogger, NULL);

    pthread_t clientThread;     //creates client thread
    pthread_create(&clientThread, NULL, threadClient, NULL);


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

    pthread_join(workerThread, NULL);
    pthread_join(loggerThread, NULL);
    pthread_join(clientThread, NULL);

    return 0;
}

void *threadWorker(void *arg) {

    while (1) {         // dequeue connection worker
        pthread_mutex_lock(&connection_mutex);
        while (connection_count == 0) {
            pthread_cond_wait(&connection_full, &connection_mutex)
        }
        int socket_descriptor = connection_queue[connection_front].socket_descriptor;       //connection queue
        connection_front = (connection_front + 1) % MAX_BUFFER_SIZE;
        connection_count--;

        pthread_mutex_unlock(&connection_mutex);

        char bufffer[1024];         //spell checking with client

        ssize_t bytesReceived = recv(socket_descriptor, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            close(socket_descriptor);
            continue;
        }

        const char *response = "green";         // implement spell checking
        send(socket_descriptor, response, strlen(response), 0);

        pthread_mutex_lock(&log_mutex);                         //queue log message
        snprintf(loq_queue[++log_rear].message, sizeof(loq_queue[log_rear].message, 
        "Socket %d: %s", socket_descriptor, response);

        long_count++;                           //increment log count
        pthread_cond_signal(&log_full);
        pthread_mutex_unlock(&log_mutex);
    }
    return NULL;

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
