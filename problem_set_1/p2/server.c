#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

/*
Write a simple C program that creates and initializes a server socket.
Once initialized, the server should accept a client connection, close
the connection, and then exit.
*/

#define DEFAULT_PORT "8080"  // convention for alternative http
#define BACKLOG 10
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))  // do not use with pointers :)


int main(int argc, char *argv[]) {

    // print connection details
    printf("Preparing server on port: %s\n", DEFAULT_PORT);

    // resolve server address
    struct addrinfo hints, *res, *p;
    int status;
    int socketfd, clientfd;
    int tries = 0;

    // servers need an extra, protocol(IP)-agnostic data structure to recieve
    // the connecting client's address. It's literally just like a block of
    // memory which is the same size as the greater of either sockaddr type
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);

    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_protocol = 0;  // always use 0. this is coulped with family
    hints.ai_flags = AI_PASSIVE;  // fill in my IP automatically

    if ((status = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo failed with code: %s\n", gai_strerror(status));
        return status;
    }

    // now iteratively try to create a socket and bind to it
    for (p = &res[0]; p != NULL; p = p->ai_next) {
        tries++;
        //                     domain=IPv4  type=socktype    protocol=same as domain really
        if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            // socket creation failed
            continue;
        }
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == 0) {
            // success! break
            break;
        }
    }

    if (p == NULL) {
        // couldn't bind to anything
        fprintf(stderr, "unable to bind to any of %d returned addresses\n", tries);
        return 2;
    }

    // hooray we are connected!
    printf("We are live with socketfd %d!\n", socketfd);
    listen(socketfd, BACKLOG);

    // Accept incoming connection
    if ((clientfd = accept(socketfd, (struct sockaddr*) &client_addr, &addr_size)) == -1) {
        fprintf(stderr, "failed to accept connection: %s\n", strerror(errno));
        return 3;
    }

    printf("Connection accepted! Socket %d, Client %d\n", socketfd, clientfd);
    return 0;
}