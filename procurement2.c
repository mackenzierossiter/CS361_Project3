/* Procurement Client

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

/*------------------------------------------------------------------------
* Error Handling Functions
*----------------------------------------------------------------------*/
void err_sys(const char* msg)
{
fflush( stderr ) ;
perror(msg);
exit(1);
}

/*------------------------------------------------------------------------
* procurement client
*----------------------------------------------------------------------*/

int main( int argc , char *argv[] ) {
    if ( argc < 4 )
    {
        fprintf(stderr, "usage: %s orderSize ip_address port \n" , argv[0] );
        exit(1);
    }
    
    int orderSize = atoi(argv[1]);
    char* ipAdr = argv[2]; //need to figure out the type somewhere in lab
    unsigned short portNum = (unsigned short) atoi(argv[3]);

    //set up udp


    //send request message over udp to factory udp

}