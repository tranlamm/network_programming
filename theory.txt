#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



-------------------------------------------------------------------------------------

const: 
    AF_INET: sin_family of Ipv4
    AF_INET6: sin_family of Ipv6
    AF_UNSPEC: khong xac dinh
    INADDR_ANY: random ip

struct in_addr: struct of ip address
    in_addr_t s_addr (typeof(in_addr_t): ip address in uint_32_t) 

struct sockaddr: struct of general socket ip
struct sockaddr_in: struct of socket ipv4
    uint8_t sin_len: sizeof(sockaddr_in) 16 bytes
    sa_family_t sin_family: ho dia chi IP
    in_port_t sin_port: port number
    struct in_addr sin_addr
struct sockaddr_in6: struct of socket ipv6

-------------------------------------------------------------------------------------

function:
    IPv4:
        in_addr_t inet_addr(const char *c): string to in_addr_t ipv4 address
        int inet_aton(const char *c, struct in_addr *inp): string to struct in_addr
        char *inet_ntoa(struct in_addr): struct in_addr to string
    Network order: big-endian
    Host order: little-endian
        uint32_t htonl(uint32_t hostlong): little->big
        uint16_t htons(uint16_t hostshort): little->big
    
        uint32_t ntohl
        uint16_t ntohs

DNS:
    int getaddrinfo(
        const char* nodename: domain name
        const char* servname: typeof service (http, ftp, smtp, ...) or port number
        const struct addrinfo* hints
        struct addrinfo** res
    )

    struct addrinfo: linked-list holds informations about domain name
        int ai_flags
        int ai_family
        int ai_socktype
        int ai_protocol
        socklen_t ai_addrlen: chieu dai ai_addr
        char* ai_canonname: ten mien
        struct sockaddr* ai_addr: cau truc dia chi socket ip
        struct addrinfo* ai_next: con tro toi cau truc sau

-------------------------------------------------------------------------------------