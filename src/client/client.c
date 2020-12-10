
#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "murmur3.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
//your application name -port proxyportnumber filename
static void usage()
{
    extern char * __progname;
    fprintf(stderr, "usage: %s ipaddress portnumber proxyportnumber object\n", __progname);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_sa;
    char buffer[80], *ep;
    size_t maxread;
    ssize_t r, rc;
    u_short port;
    u_long p;
    int sd;
    
    if (argc != 4)
        usage();
    
    p = strtoul(argv[2], &ep, 10);
    if (*argv[2] == '\0' || *ep != '\0') {
        /* parameter wasn't a number, or was empty */
        fprintf(stderr, "%s - not a number\n", argv[2]);
usage();
}
if ((errno == ERANGE && p == ULONG_MAX) || (p > USHRT_MAX)) {
    /* It's a number, but it either can't fit in an unsigned
     * long, or is too big for an unsigned short
     */
    fprintf(stderr, "%s - value out of range\n", argv[2]);
    usage();
}
/* now safe to do this */
port = p;
const int SIZE = 6;
const char *proxyname[SIZE];
proxyname[0] = "P1";proxyname[1] = "P2";proxyname[2] = "P3";proxyname[3] = "P4";proxyname[4] = "P5";proxyname[5] = "P6";
/// my code: hrw hash
uint32_t hash[4];                // Output for the hash
uint32_t seed = 42;              // Seed value for hash
largest=MurmurHash3_x86_32(strcat(argv[4],proxyname[0]), strlen(strcat(argv[4],proxyname[0])), seed, hash);  //concatentaing and calculating hash
for (i = 1; i < SIZE; i++)
{
if (largest < MurmurHash3_x86_32(strcat(argv[4],proxyname[i]), strlen(strcat(argv[4],proxyname[i])), seed, hash);)
largest = MurmurHash3_x86_32(strcat(argv[4],proxyname[i]), strlen(strcat(argv[4],proxyname[i])), seed, hash);
    }//determining which proxy to ask    
    //cache:https://github.com/sonertari/SSLproxy/blob/master/src/cache.c
//TLS handshake with the proxy    https://aticleworld.com/ssl-server-client-using-openssl-in-c/

SSL_library_init();
portnum = largest;
ctx = InitServerCTX();        /* initialize SSL */
LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
proxy = OpenListener(atoi(portnum));    /* create proxy socket */
while (1)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    SSL *ssl;
    int client = accept(proxy, (struct sockaddr*)&addr, &len);  /* accept connection */
    printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    ssl = SSL_new(ctx);              /* get new SSL state with context */
    SSL_set_fd(ssl, client);      /* set connection socket to SSL state */
    Servlet(ssl);         /* service connection */
}
close(proxy);          /* close proxy socket */
SSL_CTX_free(ctx);         /* release context */
return(0);
}


