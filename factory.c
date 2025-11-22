//---------------------------------------------------------------------
// Assignment : PA-03 UDP Single-Threaded Server
// Date       :
// Author     : Gwen Lumsden and Mackenzie Rossiter
// File Name  : factory.c
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

#define MAXSTR     200
#define IPSTRLEN    50

typedef struct sockaddr SA ;

int minimum( int a , int b)
{
    return ( a <= b ? a : b ) ; 
}

void subFactory( int factoryID , int myCapacity , int myDuration ) ;

void factLog( char *str )
{
    printf( "%s" , str );
    fflush( stdout ) ;
}

/*-------------------------------------------------------*/

// Global Variable for Future Thread to Shared
int   remainsToMake , // Must be protected by a Mutex
      actuallyMade ;  // Actually manufactured items

int   numActiveFactories = 1 , orderSize ;

int   sd ;      // Server socket descriptor
struct sockaddr_in  
             srvrSkt,       /* the address of this server   */
             clntSkt;       /* remote client's socket       */

unsigned int alen = sizeof(clntSkt) ;



//------------------------------------------------------------
//  Handle Ctrl-C or KILL 
//------------------------------------------------------------

void goodbye(int sig) 
{
    /* Mission Accomplished */
    printf( "\n### I (%d) have been nicely asked to TERMINATE. "
           "goodbye\n\n" , getpid() );  
    switch( sig ) {
        case SIGTERM:
            printf("nicely asked to TERMINATE by SIGTERM ( %d ).\n" , sig ) ;
            break ;
        
        case SIGINT:
            printf("INTERRUPTED by SIGINT ( %d )\n" , sig ) ;
            break ;

        default:
            printf("unexpectedly SIGNALed by ( %d )\n" , sig ) ;
    }
    exit(0);
}

/*-------------------------------------------------------*/
int main( int argc , char *argv[] )
{
    // create vars
    char  *myName = "Kenzie&Gwen" ; 
    unsigned short port = 50015 ;      /* service port number  */
    int    N = 1 ;                     /* Num threads serving the client */

    // print name
    printf("\nThis is the FACTORY server developed by %s\n\n" , myName ) ;

    // print eid and start time
    char myUserName[30] ;
    getlogin_r ( myUserName , 30 ) ;
    time_t  now;
    time( &now ) ;
    fprintf( stdout , "Logged in as user '%s' on %s\n\n" , myUserName ,  ctime( &now)  ) ;
    fflush( stdout ) ;


    // Handle signals
    sigactionWrapper ( SIGINT ,  goodbye ) ;
    sigactionWrapper ( SIGTERM,  goodbye ) ;
	
    
    // handle cmd line args
    switch (argc) 
	{
      case 1:
        break ;     // use default port with a single factory thread
      
      case 2:
        N = atoi( argv[1] ); // get from command line
        port = 50015;            // use this port by default
        break;

      case 3:
        N    = atoi( argv[1] ) ; // get from command line
        port = atoi( argv[2] ) ; // use port from command line
        break;

      default:
        printf( "FACTORY Usage: %s [numThreads] [port]\n" , argv[0] );
        exit( 1 ) ;
    }




    // creating buffers needed for socket
    char buf  [ MAXSTR ] ;
    char ipStr[ IPSTRLEN ] ;


    // create the socket file descriptor
    sd = socket( AF_INET, SOCK_DGRAM, 0) ;
    if ( sd < 0 ) {
        err_sys("Could NOT create socket") ;
    }


    // Prepare the server's socket address structure
    memset( (void *) & srvrSkt , 0 , sizeof( srvrSkt ) );
    srvrSkt.sin_family = AF_INET;
    srvrSkt.sin_addr.s_addr = htonl( INADDR_ANY );
    srvrSkt.sin_port = htons( port ) ;


    // bind the server to above socket
    if ( bind( sd, (SA *) & srvrSkt , sizeof(srvrSkt) ) < 0 )
    {
        snprintf( buf, MAXSTR, "Could NOT bind to port %d", port );
        err_sys( buf ) ;
    }

    // bounding the socket to the IP and Port
    inet_ntop( AF_INET, (void *) & srvrSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;
    

    printf("Bound socket %d to IP %s Port %d\n", sd, ipStr, port);

    int forever = 1;
    while ( forever )
    {
        printf( "\nFACTORY server waiting for Order Requests\n" ) ; 
        alen = sizeof(clntSkt) ;

        msgBuf msg1;

        if ( recvfrom( sd , &msg1 , sizeof(msg1) , 0 , (SA *) & clntSkt , & alen ) < 0 )
        {
            err_sys( "recvfrom" ) ;
        }
            
        printf("\n\nFACTORY server received: " ) ;
        printMsg( & msg1 );  puts("");

        
        // get IP and port from sender
        inet_ntop( AF_INET, (void *) & clntSkt.sin_addr.s_addr , ipStr , IPSTRLEN ) ;
        printf("\t\tfrom IP %s Port %d\n" , ipStr , ntohs( clntSkt.sin_port ) ) ;


        // send an order confirm message
        msg1.purpose = htonl(ORDR_CONFIRM) ;
        msg1.numFac  = htonl (N) ;
        remainsToMake = htonl (msg1.orderSize) ;
        
        if (sendto( sd , &msg1 , sizeof(msg1) , 0 , (SA *) & clntSkt , alen ) < 0 )
        {
            err_sys( "recvfrom" ) ;
        }

        printf("\n\nFACTORY sent this Order Confirmation to the client " );
        printMsg(  & msg1 );  puts("");
        
        subFactory( 1 , 50 , 350 ) ;  // Single factory, ID=1 , capacity=50, duration=350 ms
    }


    return 0 ;
}

void subFactory( int factoryID , int myCapacity , int myDuration )
{
    char    strBuff[ MAXSTR ] ;   // snprint buffer
    int     partsImade = 0   ; 
    int     myIterations = 0 ;
    msgBuf  msg2;

    int toMake;

    while ( 1 )
    {
        // See if there are still any parts to manufacture
        if ( remainsToMake <= 0 )
            break ;   // Not anymore, exit the loop
        
        
        // how many to make
        toMake = minimum(remainsToMake, myCapacity) ;

        printf("Factory #  %d: Going to make    %d parts in  %d mSec\n", factoryID, toMake, myDuration) ;
        // update remainToMake
        remainsToMake -= toMake;
        
        // sleep to simulate making items
        int sleep_time = myDuration * 1000 ;
        usleep( sleep_time ) ;
        partsImade += toMake ;
        



        // Send a Production Message to Supervisor
        msg2.purpose   = htonl ( PRODUCTION_MSG) ;
        msg2.facID     = htonl ( factoryID )     ;
        msg2.capacity  = htonl ( myCapacity )    ;
        msg2.duration  = htonl ( myDuration )    ;
        msg2.partsMade = htonl ( partsImade )    ;

        if (sendto( sd , &msg2 , sizeof(msg2) , 0 , (SA *) & clntSkt , alen ) < 0 )
        {
            err_sys( "recvfrom" ) ;
        }
        printf("\nline 249, production message was sent\n") ;
       
        myIterations ++ ;
    }

    
    
    // Send a Completion Message to Supervisor
    msg2.purpose = COMPLETION_MSG ;
    sendto( sd , &msg2 , sizeof(msg2) , 0 , (SA *) & clntSkt , alen ) ;


    snprintf( strBuff , MAXSTR , ">>> Factory # %-3d: Terminating after making total of %-5d parts in %-4d iterations\n" 
          , factoryID, partsImade, myIterations);
    factLog( strBuff ) ;
    
}

