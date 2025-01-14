/*
NOTES:
- Type: SOCK_STREAM: Stream socket, reliable 2-way connected stream using "Transmission Control Protocol" (TCP)
    - telnet, ssh, http
- Type: SOCK_DGRAM: Datagram socket, unreliable, but self-contained packet using "User Datagram Protocol" (UDP)
    - video, audio, games, tftp, dhcpcd
    - tftp has another protocol on top of UDP which requires an ACK

Data Encapsulation:
    Application -> Presentation -> Session -> Transport -> Network -> Data Link -> Physical
    telnet                                    TCP, IP      ethernet

Convert endianness for shorts and longs for both incoming and outgoing
    - htons(): host to network short
    - htonl(): host to network long
    - ntohs(): network to host short
    - ntohl(): network to host long

struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6  --> we use sockaddr_in, or sockaddr_in6, but they can both be cast to this
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};

---------- IPv4 sock addr ----------
struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr; set to 0s with memset()
};

struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};
------------------------------------

---------- IPv6 sock addr ----------
struct sockaddr_in6 {
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information     (advanced, out of scope)
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID                  (advanced, out of scope)
};

struct in6_addr {
    unsigned char   s6_addr[16];   // IPv6 address
};
------------------------------------

struct sockaddr_storage { // helper which is big enough to hold sockaddr_in or sockaddr_in6, in case we need to
    sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};

To convert IP address (presentation/printable to network)
struct sockaddr_in sa; // IPv4
struct sockaddr_in6 sa6; // IPv6
inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // check output -1 for error!
inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &sa6.sin6_addr) // check output -1 for error!

Reverse of that
char ip4[INET_ADDRSTRLEN];
struct sockaddr_in sa;
inet_ntop(AF_INET, &(sa.sin_addr), ip4, INETADDRSTRLEN);
// do the same thing with INET6_ADDRSTRLEN


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);

Scenario	                            Returned Results	                                                        Use Case
Provide node and service	            Resolved addresses for the given hostname and port/service.	                Client connections or specific bindings.
Set hint.ai_flags (e.g., AI_PASSIVE)	Wildcard addresses for all interfaces.	                                    Server sockets or general-purpose bindings.
Both node/service and ai_flags	        Resolved addresses with additional query behavior (e.g., canonical names).	Advanced queries requiring both resolution and flags.

*/

/*
Write a simple C program that creates, initializes, and connects a client socket
to a server socket. You should provide a way to specify the connecting server
address and port. This can be hardcoded or passed via the command line.
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // cast inputs to node address
    // char address_str[] = "www.example.com";
    // char port_str[] = "3490";

    struct addrinfo hints, *servinfo, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: main <hostname>\n");
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // dont care if IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    // hints.ai_flags = AI_PASSVE; // fills in my IP - not needed for client, only server

    if ((status = getaddrinfo(argv[1], NULL, &hints, &servinfo)) != 0) {  // we are not specifying a port/service. Why not?
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        return 2;
    }
    // serveinfo now points to a linked list of 1 or more struct addrinfos
    printf("IP addresses for %s:\n\n", argv[1]);

    // iterate through result addresses
    for (p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself
        // different fields in IPv4 and IPv6
        if (p->ai_family == AF_INET) {  // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {  // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        
        // convert the IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf("\t%s: %s\n", ipver, ipstr);
    }
    freeaddrinfo(servinfo);
    return 0;
}