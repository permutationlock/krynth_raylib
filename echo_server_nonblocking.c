#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // contains read/write sycalls

#include <sys/socket.h> // contains socket, connect, accept syscalls

#include <netinet/in.h> // needed for sockaddr_in and htonl functions
#include <arpa/inet.h> // needed for inet_ntoa

#include <poll.h> // needed for poll and struct pollfd

#include <errno.h> // needed for errno global

enum {
  PORTNUM = 2300,
  MAXCONS = 5,
  MAXRECVLEN = 500,
  QUEUELEN = 8
};

struct message {
    char buffer[MAXRECVLEN];
    int length;
};

struct message_queue {
    struct message data[QUEUELEN];
    int first;
    int last;
};

void pop_queue(struct message_queue* queue) {
    memset(queue->data + queue->first, 0, sizeof(struct message));
    queue->first = (queue->first + 1) % QUEUELEN;
}

void push_queue(struct message_queue* queue) {
    queue->last = (queue->last + 1) % QUEUELEN;
}

struct message* front_queue(struct message_queue* queue) {
    return queue->data + queue->first;
}

struct message* back_queue(struct message_queue* queue) {
    return queue->data + queue->last;
}

struct pollfd connection_fds[MAXCONS+1];
struct message_queue send_queue[MAXCONS];

int send_message(int con_index) {
    struct message* message;
    int len = 0;
    int confd = connection_fds[con_index].fd;
    struct message_queue* queue = &(send_queue[con_index]);

    while(queue->first != queue->last) {
        message = front_queue(queue);
        len = send(confd, message->buffer, message->length, 0);

        if(len < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) return 1;
            return 0;
        } if(len == 0) {
            return 0;
        }

        printf("sent client %d: %s\n", confd, message->buffer);
        pop_queue(queue);
    }

    return 1;
}

int handle_connection(int con_index) {
    int len = 0;
    int confd = connection_fds[con_index].fd;
    struct message_queue* queue = &(send_queue[con_index]);
    struct message* message = back_queue(queue);

    len = recv(confd, message->buffer, MAXRECVLEN, 0);
    
    if(len < 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK) return 1;
        return 0;
    } if(len == 0) {
        return 0;
    }

    message->length = len;
    message->buffer[len] = '\0';

    printf("client %d sent: %s\n", con_index, message->buffer);

    push_queue(queue);

    return send_message(con_index);
}

int accept_connection(int socketfd) {
    // stores size of address (will be set to size of connection address)
    socklen_t socksize = sizeof(struct sockaddr_in);

    // socket info about the machine connecting to us
    struct sockaddr_in dest;

    int index = 1;
    for(; index < MAXCONS+1; ++index) {
        if(connection_fds[index].fd < 0) break;
    }
    if(index == MAXCONS+1) return 0;

    // accept a single connection on the socket fd mysocket
    connection_fds[index].fd = accept4(
        socketfd,
        (struct sockaddr *)&dest,
        &socksize,
        SOCK_NONBLOCK
    );

    memset(&(send_queue[index]), 0, sizeof(struct message_queue));

    printf("Incoming connection from %s assigned to %d at index %d\n",
        inet_ntoa(dest.sin_addr), connection_fds[index].fd, index);

    return 1;
}

// clean up data and close client connection
void clean_connection(int index) {
    printf("closing connection %d\n", index);
    close(connection_fds[index].fd);
    connection_fds[index].fd = -1;
}

int main(int argc, char *argv[])
{
    // initialize array to blank memory
    memset(connection_fds, 0, sizeof(struct pollfd) * (MAXCONS + 1));

    // set all file descriptors to look for input and start ignored
    for(int i=0; i < MAXCONS + 1; ++i) {
        connection_fds[i].fd = -1;
        connection_fds[i].events = POLLIN;
    }

    // struct to store the address and socket of our server
    struct sockaddr_in serv;

    // zero the struct before filling the fields
    memset(&serv, 0, sizeof(serv));

    // set the type of connection to TCP/IP
    serv.sin_family = AF_INET;                

    // set our address to accept connections on any interface
    // htonl is named for Host TO Network Long
    //  -> it reverses the bit order of a long's binary representation
    serv.sin_addr.s_addr = htonl(INADDR_ANY); 

    // set the server port number
    // htons is named for Host TO Network Short
    // -> it works the same as htonl but for short (16 bit) numbers
    serv.sin_port = htons(PORTNUM);

    //socket used to listen for incoming connections
    int mysocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); 

    // bind serv information to mysocket
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr_in));

    // start listening, allowing up to MAXCONS connections
    listen(mysocket, MAXCONS);

    // set the first file descriptor to mysocket
    connection_fds[0].fd = mysocket;

    int num_events;
    while(1) {
        // wait for any of the following events:
        //   a new client connection is opened
        //   an open connection sends a message
        //   a previously blocked socket becomes writeable
        num_events = poll(connection_fds, MAXCONS+1, 10000);

        // kill program if an error occurs (should never happen)
        if(num_events < 0) exit(1);

        if(num_events > 0) {
            // accept any new connections
            if(connection_fds[0].revents !=0) {
                accept_connection(connection_fds[0].fd);
            }

            // handle client connection events
            for(int i = 1; i < MAXCONS+1; ++i) {
                if((connection_fds[i].revents & POLLIN) != 0) {
                    if(handle_connection(i) < 1) {
                        clean_connection(i);
                    }
                } else if((connection_fds[i].revents & POLLOUT) !=0) {
                    if(send_message(i) < 1) {
                        clean_connection(i);
                    }
                }
            }
        }
    }
    
    for(int i = 1; i < MAXCONS+1; ++i) {
        if(connection_fds[i].fd > 0) {
            close(connection_fds[i].fd);
        }
    }

    close(mysocket);
    return EXIT_SUCCESS;
}