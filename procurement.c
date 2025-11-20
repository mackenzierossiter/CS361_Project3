//---------------------------------------------------------------------
// Assignment : PA-03 UDP Single-Threaded Server
// Date       :
// Author     : WRITE YOUR  NAME(S)  HERE  ... or risk losing points
// File Name  : procurement.c
//---------------------------------------------------------------------

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "wrappers.h"
#include "message.h"

#define MAXFACTORIES    20

typedef struct sockaddr SA ;

/*-------------------------------------------------------*/
int main( int argc , char *argv[] )
{
    int     numFactories ,      // Total Number of Factory Threads
            activeFactories ,   // How many are still alive and manufacturing parts
            iters[ MAXFACTORIES+1 ] = {0} ,  // num Iterations completed by each Factory
            partsMade[ MAXFACTORIES+1 ] = {0} , totalItems = 0;

    char  *myName = "Kenzie&Gwen" ; 
    printf("\nPROCUREMENT: Started. Developed by %s\n\n" , myName );    

    char myUserName[30] ;
    getlogin_r ( myUserName , 30 ) ;
    time_t  now;
    time( &now ) ;
    fprintf( stdout , "Logged in as user '%s' on %s\n\n" , myUserName ,  ctime( &now)  ) ;
    fflush( stdout ) ;
    
    if ( argc < 4 )
    {
        printf("PROCUREMENT Usage: %s  <order_size> <FactoryServerIP>  <port>\n" , argv[0] );
        exit( -1 ) ;  
    }

    unsigned        orderSize  = atoi( argv[1] ) ;
    char	       *serverIP   = argv[2] ;
    unsigned short  port       = (unsigned short) atoi( argv[3] ) ;
 

    /* Set up local and remote sockets */
    int sd, n;

    //allocate a socket
    sd = socket( AF_INET, SOCK_DGRAM, 0) ;
    if (sd < 0) {
        err_sys("Could NOT create socket" ) ;
    }


    // Prepare the server's socket address structure
    struct sockaddr_in srvSkt ; /* Server's socket structrue */
    memset( (void *) & srvSkt , 0 , sizeof( srvSkt ) );
    srvSkt.sin_family = AF_INET;
    srvSkt.sin_port = htons( port ) ;
    if( inet_pton( AF_INET, serverIP , (void *) & srvSkt.sin_addr.s_addr ) != 1 )
        err_sys( "Invalid server IP address" ) ;

    // Send the initial request to the Factory Server
    msgBuf  msg1;

    msg1.purpose = REQUEST_MSG;
    msg1.orderSize = orderSize;
    msg1.factoryID = 0;
    msg1.capacity = 0;
    msg1.numItemsMade = 0;
    msg1.duration = 0;

    if ( sendto(sd, &msg1, sizeof(msg1), 0, (SA *) &srvSkt, sizeof(srvSkt)) < 0){
        err_sys("procurement sendto failed");
    }


    printf("\nPROCUREMENT Sent this message to the FACTORY server: "  );
    printMsg( & msg1 );  puts("");


    /* Now, wait for oreder confirmation from the Factory server */
    msgBuf  msg2;
    printf ("\nPROCUREMENT is now waiting for order confirmation ...\n" );


    unsigned int alen = sizeof(srvSkt);
    if (recvfrom(sd, &msg2, sizeof(msg2), 0, (SA *) &srvSkt, &alen) < 0) {
        err_sys("procurement recvfrom failed");
    }

    printf("PROCUREMENT received this from the FACTORY server: "  );
    printMsg( & msg2 );  puts("\n");


    if (msg2.purpose == ORDR_CONFIRM) {
        activeFactories = msg2.numFac;
        numFactories = msg2.numFac;
    } else {
        err_sys("not a order message");
    }
    


    // Monitor all Active Factory Lines & Collect Production Reports
    while ( activeFactories > 0 ) // wait for messages from sub-factories
    {
        msgBuf msg3;

        alen = sizeof(srvSkt);
        if (recvfrom(sd, &msg3, sizeof(msg3), 0, (SA *) &srvSkt, sizeof(srvSkt)) < 0) {
            err_sys("procurement recvfrom failed");
        }

       // Inspect the incoming message

       if (msg3.purpose == PRODUCTION_MSG) {
            printMsg( & msg3 );  puts("\n");
            iters[msg3.facID]++;
            partsMade[msg3.facID] += msg3.partsMade;
       } else if (msg3.purpose == COMPLETION_MSG) {
            activeFactories--;
       }
    } 

    // Print the summary report
    totalItems  = 0 ;
    printf("\n\n****** PROCUREMENT Summary Report ******\n");

    for (int i = 1; i <= numFactories; i++) {
        printf("Factory #   %d made a total of %5d in %5d\n", i, partsMade[i], iters[i]);
        totalItems += partsMade[i];
    }

    printf("==============================\n") ;


    printf("Grand total parts made = %5d    vs  order size of %5d\n", totalItems, orderSize);


    printf( "\n>>> PROCUREMENT Terminated\n");


    close(sd);


    return 0 ;
}
