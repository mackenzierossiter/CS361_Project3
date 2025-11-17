/* Factory Server

Gwen Lumsden and Mackenzie Rossiter

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define MAXBUFLEN 256
#define IPSTRLEN 50
typedef struct sockaddr SA ;
//need to add sigint and sigterm handling
/*------------------------------------------------------------------------
* Error Handling Functions
*----------------------------------------------------------------------*/
void err_sys(const char* msg)
{
    fflush( stderr ) ;
    perror( msg ) ;
    exit( 1 );
}

int main(int argc, char *argv[])
{
    struct sockaddr_in
        srvrSkt , /* the address of this server */
        clntSkt ; /* remote client's socket */
    unsigned short port = 13 ; /* service name or port number */
    int sd ; /* server socket */
    char buf[ MAXBUFLEN ] ; /* "input" buffer; any size > 0 */
    char ipStr[ IPSTRLEN ] ; /* dotted-dec IP addr. */

    unsigned int alen; /* from-address length */

    sd = socket( AF_INET, SOCK_DGRAM, 0) ;
    if ( sd < 0 ) {
        err_sys("Could NOT create socket") ;
    } 

    //honesty have no clue whats going on here i just copy pasted from lab
    //kinda get it now?

    // Prepare the server's socket address structure
    memset( (void *) & srvrSkt , 0 , sizeof( srvrSkt ) );
    srvrSkt.sin_family = AF_INET;
    srvrSkt.sin_addr.s_addr = htonl( INADDR_ANY );
    srvrSkt.sin_port = htons( port ) ;

    // Now, bind the server to above socket
    if ( bind( sd, (SA *) & srvrSkt , sizeof(srvrSkt) ) < 0 )
    {
        snprintf( buf, MAXBUFLEN, "Could NOT bind to port %d", port );
        err_sys( buf ) ;
    }

    inet_ntop( AF_INET, (void *) & srvrSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;
    printf( "Bound socket %d to IP %s Port %d\n" , sd , ipStr , ntohs( srvrSkt.sin_port ) );

    while (1) {
        alen = sizeof(clntSkt) ;
        fprintf( stderr, "FACTORY server waiting for Order Requests") ;

        //receive from client
        if ( recvfrom( sd , buf , MAXBUFLEN , 0 , (SA *) & clntSkt , & alen ) < 0 )
            err_sys( "recvfrom" ) ;
            
        //got lost here
        fprintf( stderr, "FACTORY server received: { %s, OrderSize= %s}", buf, )
    }

}