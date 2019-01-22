#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include <sys/socket.h>
#include <getopt.h>
#include "csapp.h"
#include <pthread.h>
#include "server.h"
#include <signal.h>


static void terminate(int status);

struct client_registry{
    int fd;
    pthread_t thread_id;
    struct client_registry *next;
};



CLIENT_REGISTRY *client_registry;

extern pthread_mutex_t trans_creation_mutex;
extern int curr_transaction_no;

void sighup_handler (int signum)
{
    //printf(" \n\n\n  Printing : \n\n\n");
    terminate(EXIT_SUCCESS); //EXIT_FAILURE
}


int get_no(char *str){
    int len = strlen(str);
    int no = 0;
    for(int i = 0; i < len ; i++){
        char c = str[i];
        if( c < '0' || c > '9')
            return -1;
        int digit = (int)(c-'0');
        no = no * 10 + digit;
        if( no > 65535)
            return -1;
    }
    return no;
}


int main(int argc, char* argv[]){
    int listenfd, connfd;// host_name;
    struct client_registry registry_node;
    client_registry = &registry_node;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char optval;
    char *str = NULL ;
    //char *str2 = NULL ;
    //pthread_t tid;
    int flag_q = 0 , flag_p = 0 , flag_h = 0;

    trans_init();


    /* Installing the SIGHUP handler */
    struct sigaction new_action;//, old_action;
    new_action.sa_handler = sighup_handler;
    sigemptyset (&new_action.sa_mask);
    // sigaddset(&new_action.sa_mask, SIGHUP);
    new_action.sa_flags = 0;
    sigaction (SIGHUP, NULL, NULL);
    //if (old_action.sa_handler != SIG_IGN)
    //    sigaction (SIGHUP, &new_action, NULL);
    //char file_name[100];
    //-----------Installing signal handler----------------------//
    //signal(SIGPIPE, SIG_IGN);
    //------------------------------------------------//

    while(optind < argc) {
        debug("optind : %d and argc : %d", optind, argc);
        if((optval = getopt(argc, argv, "p:h:q")) != -1) {
            debug("optval : %c" , optval);
            switch(optval) {
                case 'q':
                    debug("Expecting input file ");
                    if(flag_q == 1){
                        debug("Need to implement : Multiple q flag");
                        exit(EXIT_FAILURE);
                    }
                    flag_q = 1;
                    break;
                case 'p':
                    debug("Port Number");
                    if(flag_p == 1){
                        debug("Need to implement : Multiple p flag");
                        exit(EXIT_FAILURE);
                    }
                    if(optarg == NULL){
                        debug("opt arg is NULL exiting");
                        exit(EXIT_FAILURE);
                    }
                    str = optarg;
                    debug( " port : %s",str);
                    flag_p = 1;
                    break;
                case 'h':
                    debug("Host name ");
                    if(flag_h == 1){
                        // Todo
                        debug("Need to implement : Multiple q flag");
                        exit(EXIT_FAILURE);
                    }
                    if(optarg == NULL){
                        debug("opt arg is NULL exiting");
                        exit(EXIT_FAILURE);
                    }
                    //str2 = optarg;
                    //debug( " host name : %s",str2);
                    flag_h = 1;
                    break;
                case '?':
                    debug("Invalid argument");
                    exit(EXIT_FAILURE);
                default:
                    debug("Should never happen : default hit in switch");
                    exit(EXIT_FAILURE);

            }

        }
    }

    if( flag_p == 0 || str == NULL){
        debug("No port no");
        exit(EXIT_FAILURE);
    }
    // Todo Get_no should implement_long
    client_registry = creg_init();
    trans_init();
    store_init();


    int port_no = get_no(str);
    debug("port_no : %d",port_no);
    listenfd = Open_listenfd(port_no);
    if(listenfd < 0){
        debug("Open_listenfd failed");
        exit(EXIT_FAILURE);
    }
    debug("listen fd : %d",listenfd);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        if(Accept < 0){
            debug("Issue in Accept : must exit cleanly");
            //exit_cleanly();
        }
        debug("conn fd :  %d",(int)connfd);
        pthread_t tid;
        //TRANSACTION *tr = trans_create();
        pthread_create(&tid,NULL,xacto_client_service,(void *)&connfd);
        debug("Parent :Thread created");
        //debug("New cliend with fd  : %d",connfd);
    }




    // test
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initializations of the client_registry,
    // transaction manager, and object store.


    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    fprintf(stderr, "You have to finish implementing main() "
        "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
