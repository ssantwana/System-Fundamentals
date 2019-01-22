#include "server.h"
#include "debug.h"
#include <pthread.h>
#include "protocol.h"
#include <errno.h>
#include "transaction.h"
#include "data.h"
#include "store.h"
#include <time.h>

/*
 * Client registry that should be used to track the set of
 * file descriptors of currently connected clients.
 */
//extern CLIENT_REGISTRY *client_registry;

/*
 * Thread function for the thread that handles client requests.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.

 */
TRANS_STATUS handle_put_request(XACTO_PACKET *pkt , void *data, int fd , TRANSACTION *tr);
TRANS_STATUS handle_get_request(XACTO_PACKET *pkt , void * data, int fd , TRANSACTION *tr);
TRANS_STATUS handle_commit_request(  int fd , TRANSACTION *tr);
void handle_aborted( TRANSACTION *tr,int fd);
void handle_commit( TRANSACTION *tr,int fd);

struct timespec start,end;

void *xacto_client_service(void *arg){
    // check if a thread can detach itself
    if( pthread_detach( pthread_self() ) != 0){
        debug("thread not detached properly");
    }
    debug("In thread :");
    int fd = *((int *)arg);
    if(fd < 0){
        debug("Bad file descriptor");
        return NULL;
    }
    debug("About to free pointer\n");
    //free(arg);
    debug("Free done");


    TRANS_STATUS status  = TRANS_PENDING;
    debug("About to creat transaction");
    TRANSACTION *tr = trans_create();
    debug("transaction created");
    //int reading_fd = fd, writing_fd = fd;
    XACTO_PACKET temp;
    XACTO_PACKET *pkt = &temp;
    void *data = NULL;
    debug("About to read data");
    int err = proto_recv_packet(fd , pkt , &data);
    debug("err : %d",err);
    debug("Data read");
    if(err != 0 ){
        //thread_fail();
        debug("Thread fail : recv packet error");
        trans_abort(tr);
        return NULL;
        //status = STATUS_ABORTED;
        //return NULL;
    }
    XACTO_PACKET_TYPE pkt_type = pkt->type;
    // XACTO_PUT_PKT, XACTO_GET_PKT, XACTO_DATA_PKT, XACTO_COMMIT_PKT
    // XACTO_REPLY_PKT
    while( status != TRANS_ABORTED  && status != TRANS_COMMITTED ){
        debug("In while about to switch");
        debug("details : null : %d, pkt->size : %d ",pkt->null,pkt->size);
        //debug("pkt->type :%s",pkt_type);
        //switch(pkt_type){
            if( pkt_type == XACTO_PUT_PKT ){ status = handle_put_request(pkt,data,fd,tr);debug("put request recieved");}
            else{
                if( pkt_type == XACTO_GET_PKT ){ status = handle_get_request(pkt,data, fd,tr); debug("Packet recieved"); }
                else{
                    if( pkt_type == XACTO_COMMIT_PKT ){ status = handle_commit_request(fd,tr);debug("Handling commit transaction");}
                    else{
                        debug("Should not happen");
                    }
                }
            }
            //case XACTO_DATA_PKT : status = handle_data_pkt(data,fd , tr); debug("data recieved");break;
            //case XACTO_COMMIT_PKT : status = handle_commit_request(fd,tr);debug("Handling commit transaction");break;
            //default : debug("Should not happen");

        if( status == TRANS_ABORTED  || status  == TRANS_COMMITTED ){
            tr->status = status;
            break;
        }
        err = proto_recv_packet(fd , pkt , &data);
        if(err != 0 ){
            //thread_fail();
            debug("Thread fail : recv packet error");
            status = TRANS_ABORTED;
            //return NULL;
        }
        pkt_type = pkt->type;

    }

    if( status == TRANS_ABORTED ){
        handle_aborted(tr,fd);
        //ToDo
        return NULL;
    }

    if( status == TRANS_COMMITTED ){
        handle_commit(tr,fd);
        //ToDo
        return NULL;

    }



    return NULL;
}


TRANS_STATUS handle_put_request(XACTO_PACKET *pkt , void *data, int fd , TRANSACTION *tr){
    /*

    uint8_t type;          // Type of the packet
    uint8_t status;                // Status (in reply packet)
    uint8_t null;                  // Whether payload is NULL (in data packet)
    uint32_t size;                 // Payload size (in data packet)
    uint32_t timestamp_sec;        // Seconds field of time packet was sent
    uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent

    */

    //typedef enum { TRANS_PENDING, TRANS_COMMITTED, TRANS_ABORTED } TRANS_STATUS;
    //int err = 0;
    //debug()
    debug("In handle put request");
    debug("details : null : %d, pkt->size : %d ",pkt->null,pkt->size);
    if( pkt -> null || pkt->size == 0  || tr->status  == TRANS_ABORTED )
        return TRANS_ABORTED;

    void *data_value = NULL;
    XACTO_PACKET new_pkt;
    XACTO_PACKET *pkt_value = &new_pkt;
    debug("About to recieve data pkt");
    int err  = proto_recv_packet(fd , pkt_value , &data_value);
    debug("after recievieng : %d",err);
    if( err < 0 ) return TRANS_ABORTED;

    if( pkt_value  -> status != XACTO_DATA_PKT )
        return TRANS_ABORTED;

    if( ( pkt_value -> size == 0 && !pkt_value -> null ) || ( pkt_value -> size != 0 && pkt_value -> null ) ){
        debug("Size and null in data pkt inconsistent : ");
        return TRANS_ABORTED;
    }

    if( ( pkt_value -> size == 0 && data_value != NULL ) || ( pkt_value -> size == 0 && data_value != NULL ) ){
        debug("Data and size inconsistent");
        return TRANS_ABORTED;
    }

    BLOB *blob = blob_create((char *)data , pkt -> size );
    if( blob == NULL) return TRANS_ABORTED;

    KEY *key = key_create( blob );
    BLOB *blob_value = blob_create((char *)data_value , pkt_value -> size );

    TRANS_STATUS  status  = store_put( tr, key, blob_value);

    if( status == TRANS_ABORTED ) return status;
    // ToDo set time
    XACTO_PACKET return_pkt;
    return_pkt.type = XACTO_REPLY_PKT;
    return_pkt.status = status;
    return_pkt.null = 1;
    return_pkt.size = 0;
    return_pkt.timestamp_sec = 0;//clock_gettime(CLOCK_MONOTONIC,&start);
    return_pkt.timestamp_nsec = 0;//clock_gettime(CLOCK_MONOTONIC,&start);


     //= { XACTO_REPLY_PKT , TRANS_PENDING , 1 , 0 , 0 , 0 };

    /*
    typedef struct {
    uint8_t type;          // Type of the packet
    uint8_t status;                // Status (in reply packet)
    uint8_t null;                  // Whether payload is NULL (in data packet)
    uint32_t size;                 // Payload size (in data packet)
    uint32_t timestamp_sec;        // Seconds field of time packet was sent
    uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent
    } XACTO_PACKET;
    */
    err = proto_send_packet(fd , &return_pkt , NULL);
    if( err < 0 ) return TRANS_ABORTED;


    return status;

}


TRANS_STATUS handle_get_request(XACTO_PACKET *pkt , void *data , int fd , TRANSACTION *tr){
    /*

    uint8_t type;          // Type of the packet
    uint8_t status;                // Status (in reply packet)
    uint8_t null;                  // Whether payload is NULL (in data packet)
    uint32_t size;                 // Payload size (in data packet)
    uint32_t timestamp_sec;        // Seconds field of time packet was sent
    uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent

    */

    //typedef enum { TRANS_PENDING, TRANS_COMMITTED, TRANS_ABORTED } TRANS_STATUS;

    if(  pkt->size != 0  || data  == NULL || tr->status  == TRANS_ABORTED) {
        debug("pkt size is not 0");
        return TRANS_ABORTED;
    }

    BLOB *blob = blob_create((char *)data , pkt -> size );
    if( blob == NULL) return TRANS_ABORTED;



    KEY *key = key_create( blob );
    BLOB *blob_value = NULL;

    TRANS_STATUS status = store_get( tr, key, &blob_value );


    if( status == TRANS_ABORTED ) return status;
    // ToDo set time
    XACTO_PACKET return_pkt;
    return_pkt.type = XACTO_REPLY_PKT;
    return_pkt.status = status;
    return_pkt.null = blob_value -> size  == 0 ? 1 : 0;
    return_pkt.size = blob_value->size;
    return_pkt.timestamp_sec = 0;
    return_pkt.timestamp_nsec = 0;


    //XACTO_PACKET return_pkt = { XACTO_REPLY_PKT , status , 1 , blob_value->size , 0 , 0 };
    /*if( blob_value -> size  == 0){
        return_pkt.null = 1;
    }
    */


    /*
    typedef struct {
    uint8_t type;          // Type of the packet
    uint8_t status;                // Status (in reply packet)
    uint8_t null;                  // Whether payload is NULL (in data packet)
    uint32_t size;                 // Payload size (in data packet)
    uint32_t timestamp_sec;        // Seconds field of time packet was sent
    uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent
    } XACTO_PACKET;
    */
    void *data_value = NULL;
    if(blob_value ->size != 0 )
        data_value = (void *)blob_value->content;
    int err = proto_send_packet(fd , &return_pkt , data_value);
    if( err < 0 ) return TRANS_ABORTED;
    return status;

}




TRANS_STATUS handle_commit_request(  int fd , TRANSACTION *tr){
    if(tr->status == TRANS_ABORTED)
        return TRANS_ABORTED;
    TRANS_STATUS status = trans_commit(tr);
    return status;
}
void handle_aborted( TRANSACTION *tr,int fd){
    // TODO might need to do something for aborted
    trans_abort(tr);

    XACTO_PACKET return_pkt;
    return_pkt.type = XACTO_REPLY_PKT;
    return_pkt.status = TRANS_ABORTED;
    return_pkt.null = 1;
    return_pkt.size = 0;
    return_pkt.timestamp_sec = 0;
    return_pkt.timestamp_nsec = 0;
    //XACTO_PACKET return_pkt = { XACTO_REPLY_PKT , TRANS_ABORTED , 1 , 0 , 0 , 0 };
    proto_send_packet(fd , &return_pkt , NULL);

}
void handle_commit( TRANSACTION *tr,int fd){
    trans_commit(tr);
    tr->status = TRANS_COMMITTED;
    XACTO_PACKET return_pkt;
    return_pkt.type = XACTO_COMMIT_PKT;
    return_pkt.status = TRANS_COMMITTED;
    return_pkt.null = 1;
    return_pkt.size = 0;
    return_pkt.timestamp_sec = 0;
    return_pkt.timestamp_nsec = 0;
    //XACTO_PACKET return_pkt = { XACTO_REPLY_PKT , TRANS_COMMITTED , 1 , 0 , 0 , 0 };
    proto_send_packet(fd , &return_pkt , NULL);
}


