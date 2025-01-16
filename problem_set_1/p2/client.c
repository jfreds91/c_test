#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

/*
Write a simple C program that creates, initializes, and connects a client socket
to a server socket. You should provide a way to specify the connecting server
address and port. This can be hardcoded or passed via the command line.
*/
// https://www.codequoi.com/en/sockets-and-network-programming-in-c/

#define DEFAULT_SERVER "127.0.0.1"  // loopback IPv4 addr
#define DEFAULT_PORT "8080"  // convention for alternative http
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))  // do not use with pointers :)


int main(int argc, char *argv[]) {
    // default server and client
    const char *server = DEFAULT_SERVER;
    // above, const applies to the data that server points to, not the pointer itself
    // above, assigning a pointer to a character array like this is equivalent to = &DEFAULT_SERVER[0] (the array "decays" to a pointer)
    const char *port = DEFAULT_PORT;

    // override defaults if user wants
    if (argc >= 2) {
        server = argv[1];  // custom server
    }
    if (argc == 3) {
        port = argv[2];  // custom port
    }
    if (argc > 3) {
        fprintf(stderr, "usage: client [<server>] [<port>]\n");
        fprintf(stderr, "defaults: %s, %s\n", DEFAULT_SERVER, DEFAULT_PORT);
        return 1;
    }

    // print connection details
    printf("Connecting to %s:%s\n", server, port);

    // resolve server address
    struct addrinfo hints, *res, *p;
    int status;
    int socketfd;
    int tries =0;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_protocol = 0;  // always use 0. this is coulped with family

    if ((status = getaddrinfo(server, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo failed with code: %s\n", gai_strerror(status));
        return status;
    }

    // now iteratively try to create a socket and connect it to the server
    for (p = &res[0]; p != NULL; p = p->ai_next) {
        tries++;
        //                     domain=IPv4  type=socktype    protocol=default this
        if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            // socket creation failed
            continue;
        }
        if (connect(socketfd, p->ai_addr, p->ai_addrlen) == 0) {
            // success! break
            break;
        }
    }

    if (p == NULL) {
        // couldn't connect to anything
        fprintf(stderr, "unable to connect to any of %d returned addresses\n", tries);
        return 2;
    }

    // hooray we are connected!
    printf("We are live with socketfd %d!\n", socketfd);

    // now that we've knocked, it's polite to wait for them to say hello
    int recvd_len, recv_buf_size = 100;
    char recv_buf[recv_buf_size];   
    if ((recvd_len = recv(socketfd, &recv_buf, recv_buf_size - 1, 0)) == -1) {
        fprintf(stderr, "Error recv: %s\n", strerror(errno));
    } else {
        recv_buf[recvd_len] = "\0";  // ensure we null-terminate
    }
    printf("Server: %s\n", recv_buf);

    // and now we tell them something and they yell it back at us (rude)
    char greeting = "wassup wit it";
    if (send(socketfd, &greeting, strlen(&greeting), 0) == -1) {
        fprintf(stderr, "send failed: %s\n", strerror(errno));
    }
    printf("Us: %s\n", greeting);

    if ((recvd_len = recv(socketfd, &recv_buf, recv_buf_size - 1, 0)) == -1) {
        fprintf(stderr, "Error recv: %s\n", strerror(errno));
    } else {
        recv_buf[recvd_len] = "\0";  // ensure we null-terminate
    }
    printf("Server: %s\n", recv_buf);
    return 0;
}