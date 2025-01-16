#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>  // sigaction
#include <sys/wait.h>  // WNOHANG, waitpid

/*
The Echo Protocol
In C, write a server and client that implement the fictitious "echo protocol".
To implement the protocol, the server should accept any string from the client,
and then return that string with all letters capitalized (if letters exist).

Echo Protocol Example
Client sends "Hello, wOrlD"
Server echoes "HELLO, WORLD"
As soon as the server responds to a client, it may close.
And, as soon as the clients receives a response, it may close.
*/

#define DEFAULT_PORT "8080"  // convention for alternative http
#define BACKLOG 10
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))  // do not use with pointers :)

/*
When a process exits/terminates, it's state remains on the process table entry
    (e.g. IDs, state, execution context)
waitpid() is sufficient to inform the OS that we acknowledge the state of the zombie process
- wait on any (-1) child process to change state 
    - only exit of terminated, not ready/running which are not actionable by parent process
- do not bother to store it's return status (NULL)
- if none have changed state, do not block (WNOHANG)
*/ 
void sigchld_handler(int _unused) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

/*
examines the incoming sockaddr, which is version-agnostic
if IPv6, casts sa to a sockaddr_in6 pointer and returns a pointer to the sin6_addr attribute address
if IPV4, casts sa to a sockaddr_in pointer and returns a pointer to the sin_addr attribute address
*/
void *get_sin_addr_from_sockaddr(struct sockaddr *sa) {
    if (sa->sa_family==AF_INET6) {
        return &(((struct sockaddr_in6 *) sa)->sin6_addr);
    }
    return &(((struct sockaddr_in *) sa)->sin_addr);

}

int main(int argc, char *argv[]) {

    // print connection details
    printf("Preparing server on port: %s\n", DEFAULT_PORT);

    // resolve server address
    struct addrinfo hints, *res, *p;
    int status;
    int socketfd, clientfd;
    int tries = 0;
    struct sigaction sa;  // used much later to reap forks
    char ip_str_buffer[INET6_ADDRSTRLEN];  // buffer large enough to store string representation of IPv6

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

    freeaddrinfo(res);

    if (p == NULL) {
        // couldn't bind to anything
        fprintf(stderr, "unable to bind to any of %d returned addresses\n", tries);
        return 2;
    }

    // hooray we are connected!
    printf("We are are bound to socket %d! Listening...\n", socketfd);
    listen(socketfd, BACKLOG);

    // set up sigaction to reap zombie processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "sigaction");
        exit(3);
    }

    // Accept incoming connections
    while (1) {
        if ((clientfd = accept(socketfd, (struct sockaddr*) &client_addr, &addr_size)) == -1) {
            fprintf(stderr, "failed to accept connection: %s\n", strerror(errno));
            return 4;
        }

        // translate client addr into string IP for printing
        inet_ntop(client_addr.ss_family,
            get_sin_addr_from_sockaddr((struct sockaddr *)&client_addr),
            ip_str_buffer, sizeof(ip_str_buffer));
        printf("Connection accepted from %s\n", ip_str_buffer);

        if (!fork()) {  // child process
            close(socketfd);  // closes the file for the child. Does not delete it - parent can still listen!
            char *greeting = "Hey Baby Girl";
            if (send(clientfd, greeting, strlen(greeting), 0) == -1) {
                fprintf(stderr, "send failed: %s\n", strerror(errno));
            }
            printf("Child grote the parent");
            return 0;
        }

    }
    return 0;
}