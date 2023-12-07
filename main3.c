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
#define SIZE 1000000
#define MAX_LENGTH 100

struct Connection {     
    int socket_desc;
    time_t timestamp;
    int priority;
};

struct Connection connection_queue[MAX_BUFFER_SIZE];
int connection_front = 0;
int connection_rear = -1;
int connection_count = 0;
int connection_capacity = 0;

struct Log {
    char message[256];
};

struct Log log_queue[MAX_BUFFER_SIZE];
int log_front = 0;
int log_rear = -1;
int log_count = 0;
int log_capacity = 0;


pthread_mutex_t connection_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t connection_full = PTHREAD_COND_INITIALIZER;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log_full = PTHREAD_COND_INITIALIZER;

void *threadWorker(void *arg);      // processes client requests
void *threadLogger(void *arg);     //logging messages
void *threadClient(void *arg);      // sends request to the server
int compareWords(const void *a, const void *b);      // compares words
char *dictionary[SIZE];
int dictionary_size = 0;
int main() {

    int port_number = DEFAULT_PORT_NUMBER;      //assign port number
    int socket_descriptor, new_socket, c;
    struct sockaddr_in server, client;

    // creates socket function that loops, accepts and enqueues incoming connections
    //signal is sent to the worker thread when a connection is enqueued

   //FILE *dictionary_file = fopen("/usr/share/dict/words", "r");
    //if (!dictionary_file) {
     //   perror("Failed to open dictionary");
     //   exit(EXIT_FAILURE);
  //  }

    //char word[MAX_LENGTH];
    //while (fgets(word, sizeof(word), dictionary_file) != NULL) {
       // word[strcspn(word, "\n")] = 0; // Remove newline character
      //  dictionary[dictionary_size] = strdup(word); // Store a copy of the word
    //    dictionary_size++;
   // }
   // fclose(dictionary_file);

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);    //create the server socket
    if (socket_descriptor == -1) {
        puts("Error could not create socket.");
        exit(1);
    }

    // prepare sockaddr_in struct
    server.sin_family = AF_INET;            //set up server address structure
    server.sin_addr.s_addr = INADDR_ANY;            //default to 127.0.0.1
    server.sin_port = htons(port_number);       //8888


    //bind the socket to the server address
    int bind_result = bind(socket_descriptor, (struct sockaddr *)&server, sizeof(server));
    if (bind_result < 0) {
        puts("Error could not bind socket.");
        exit(1);
    }
    puts("Successfully bound to port ___.");

    // converts active socket to listening for connection
    //listen(socket_descriptor, 3);       // use listen for incoming connections
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

        listen(socket_descriptor, 3);
        //accept a new connection // create connected descriptor
        new_socket = accept(socket_descriptor, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket < 0) {
            puts("Error connection failed.");
            continue;           // continue for new connections
        }
        puts("Connection successful.");
        pthread_mutex_lock(&connection_mutex);      // lock the connection queue mutex before accessing it

        // add new connection details to the connection queue
        connection_rear = (connection_rear + 1) % MAX_BUFFER_SIZE;
        connection_queue[connection_rear].socket_desc = new_socket;
        connection_queue[connection_rear].timestamp = time(NULL);
        connection_queue[connection_rear].priority = 0;
        connection_count++;
        pthread_cond_signal(&connection_full);      // signal queue is not full
        pthread_mutex_unlock(&connection_mutex);        //unlcok the connection queue mutex
    }

    //signal if there are connections in the queue
    if (connection_count > 0) {
        pthread_cond_signal(&connection_full);
    }

    // join threads
    pthread_join(workerThread, NULL);
    pthread_join(loggerThread, NULL);
    pthread_join(clientThread, NULL);

    return 0;
}

int compareWords(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void *threadWorker(void *arg) {


    while (1) {         // dequeue connection worker
        pthread_mutex_lock(&connection_mutex);
        while (connection_count == 0) {
            pthread_cond_wait(&connection_full, &connection_mutex);
        }
        int socket_desc = connection_queue[connection_front].socket_desc;       //connection queue
        connection_front = (connection_front + 1) % MAX_BUFFER_SIZE;
        connection_count--;

        pthread_mutex_unlock(&connection_mutex);

        char buffer[1024];         //spell checking with client

        ssize_t bytesReceived = recv(socket_desc, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            close(socket_desc);
            continue;
        }


        // run spell checking
    
        char *words = strtok(buffer, " \n"); // Tokenize the buffer
        while (words != NULL) {

            char *response;
            if (bsearch(&words, dictionary, dictionary_size, sizeof(char *), compareWords)) {
                response = "OK";
            } else {
                response = "MISSPELLED";
            }

            send(socket_desc, response, strlen(response), 0);

            // Log the response
            pthread_mutex_lock(&log_mutex);
            snprintf(log_queue[++log_rear].message, sizeof(log_queue[log_rear].message), "Socket %d: %s", socket_desc, response);
            log_count++;
            pthread_cond_signal(&log_full);
            pthread_mutex_unlock(&log_mutex);

            words = strtok(NULL, " \n"); // Get next word
        }
        close(socket_desc); // Close the socket after processing
    }
    return NULL;

}

void *threadLogger(void *arg) {

    while (1) {     // dequeue the log message from the queue
        pthread_mutex_lock(&log_mutex);
        while (log_count == 0) {
            pthread_cond_wait(&log_full, &log_mutex);
        }
        char message[256];
        strcpy(message, log_queue[log_front].message);
        log_front = (log_front + 1) % MAX_BUFFER_SIZE;
        log_count--;

        pthread_mutex_unlock(&log_mutex);

        // needs log instertion
        printf("%s\n", message);
    }
return NULL;
  
}

void *threadClient(void *arg) {
    // needs client functionality
    //printf("threadClient");

    //implement sending requests
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(DEFAULT_PORT_NUMBER);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error connection failed.");
        exit(1);
    }
    const char *spellCheck = "example";
    send(client_socket, spellCheck, strlen(spellCheck), 0);

    // close client socket
    close(client_socket);

    return NULL;
}