#include "store.h"
#include "debug.h"
#include "server.h"
#include <pthread.h>
#include "protocol.h"
#include <errno.h>
#include "transaction.h"
#include "data.h"
/*
 * A transactional object store is a hash table that maps keys to values.
 * Both keys and values consist of arbitrary data, represented as "blobs".
 * At any given time, there can be any number of transactions involved in
 * concurrently executing operations on the store.  In order to manage this
 * concurrency, the store maintains, for each key, a list of "versions",
 * which represent possible values associated with that key.  The reason why
 * multiple versions are needed is because we don't know in advance which
 * transactions are going to access which keys, or whether more two transactions
 * will concurrently try to store conflicting values for the same key.
 * The version list associated with a key keeps track of the values seen by
 * all pending transactions. Each version consists of a "creator" transaction,
 * a data object, and a pointer to the next version in the list of versions for
 * the same key. The version list for each key is kept sorted by the creator
 * transaction ID, with versions earlier in the list having smaller creator IDs
 * than versions later in the list.
 *
 * A "committed version" is a version whose creator has committed.
 * Similarly, an "aborted version" is a version whose creator has aborted, and a
 * "pending version" is a version whose creator is still pending.
 * In a version list any committed versions always occur first, followed by
 * pending and aborted versions.  The committed version whose creator has the
 * greatest transaction ID represents the "current value" associated with the key.
 * Versions associated with creators that are still pending may eventually become
 * committed versions, if their creators commit.  Versions associated with creators
 * that have aborted will eventually be removed.  A given transaction may appear at
 * most once as a creator within the version list associated with any given key.
 *
 * When a transaction performs a GET or PUT operation for a particular key,
 * there is an initial "garbage collection" pass that is made over the version list.
 * In the garbage collection pass, all except the most recent committed version
 * is removed.  In addition, if any aborted version exists, then it and all later
 * versions in the list are removed and their creators are aborted.
 * After garbage collection, a version list will consist of at most one committed
 * version, followed by some number of pending versions.  Note that if there was
 * at least one committed version before garbage collection, there will be exactly
 * one committed version afterwards.  Also, if there were only aborted versions
 * before garbage collection, then the version list will be empty afterwards.
 *
 * After garbage collection, a GET or a PUT operation is only permitted to succeed
 * if the transaction ID of the performing transaction is greater than or equal to the
 * transaction ID of the creator of any existing version.
 * If this is not the case, then the operation has no effect and the performing transaction
 * is aborted.  If the transaction ID of the performing transaction is indeed the greatest,
 * then a new version is created, and that version is either added to the end of
 * the version list (if the transaction ID of the performing transaction is strictly
 * greater than the existing transaction IDs), or else it replaces the last version
 * in the list (if the performing transaction was in fact the creator of that version).
 * In addition, the performing transaction is set to be "dependent" on the creators
 * of any pending versions earlier in the list.  If the creators of any of those pending
 * versions subsequently abort, then a transaction dependent on them will also have
 * to abort.
 *
 * The value set in a new version created by PUT is the value specified as the argument
 * to PUT.  The value set in a new version created by GET is the same as the value of the
 * immediately preceding version (if any) in the version list, otherwise NULL (if there
 * is no immediately preceding version).  These rules ensure that transactions "read"
 * values left by transactions with smaller transaction IDs and they "write" values for
 * transactions with greater transaction IDs.  In database jargon, the sequence of
 * operations performed by transactions is "serializable" using the ordering of the
 * transaction IDs as the serialization order.
 */

//#include "data.h"
//#include "transaction.h"

/*
 * The overall structure of the store is that of a linked hash map.
 * A "bucket" is a singly linked list of "map entries", where each map entry
 * contains the version list associated with a single key.
 *
 * The following defines the number of buckets in the map.
 * To make things easy and simple, we will not change the number of buckets once the
 * map has been initialized, but in a more realistic implementation the number of
 * buckets would be increased if the "load factor" (average number of entries per
 * bucket) gets too high.




 */


void add_dependencies(VERSION *ver , TRANSACTION *tp);
void sortify_versions_and_garbage_collection( MAP_ENTRY *map_entry );

void free_versions_and_abort(VERSION *version);

TRANS_STATUS insert_value( TRANSACTION *tp , MAP_ENTRY *map_entry , BLOB *value);
// #define NUM_BUCKETS 8

/*
 * A map entry represents one entry in the map.
 * Each map entry contains associated key, a singly linked list of versions,
 * and a pointer to the next entry in the same bucket.

typedef struct map_entry {
    KEY *key;
    VERSION *versions;
    struct map_entry *next;
} MAP_ENTRY;


 * The map is an array of buckets.
 * Each bucket is a singly linked list of map entries whose keys all hash
 * to the same location.

struct map {
    MAP_ENTRY **table;      // The hash table.
    int num_buckets;        // Size of the table.
    pthread_mutex_t mutex;  // Mutex to protect the table.
} the_map;


 * Initialize the store.
 */
void store_init(void){
    the_map.table = malloc(NUM_BUCKETS * sizeof(void *));
    the_map.num_buckets = NUM_BUCKETS;
    pthread_mutex_init(&(the_map.mutex),NULL);
    for(int i = 0 ; i < NUM_BUCKETS ; i++){
        the_map.table[i] = NULL;
    }
}

/*
 * Finalize the store.
 typedef struct blob {
    pthread_mutex_t mutex;     // Mutex to protect reference count
    int refcnt;
    size_t size;
    char *content;
    char *prefix;              // String prefix of content (for debugging)
} BLOB;


typedef struct version {
    TRANSACTION *creator;
    BLOB *blob;
    struct version *next;
    struct version *prev;
} VERSION;

*/
void clean_blob(BLOB *blob){
    if(blob->content != NULL){
        free(blob->content);
    }
    if(blob->prefix != NULL){
        free(blob->prefix);
    }
    free(blob);
}

void version_list_fini(VERSION *curr_version){
    VERSION *next_version = NULL;
    while(curr_version != NULL){
        next_version = curr_version->next;
        clean_blob(curr_version->blob);
        free(curr_version);
        curr_version = next_version;
    }
}



void key_fini(KEY *entry){
    clean_blob(entry->blob);
    free(entry);
}

void clear_key_and_versions(MAP_ENTRY *curr_map_entry){
    key_fini(curr_map_entry->key);
    curr_map_entry->key = NULL;
    version_list_fini(curr_map_entry->versions);
    curr_map_entry->versions = NULL;
    free(curr_map_entry);

}

void store_fini(void){
    for(int i = 0; i < NUM_BUCKETS ; i++){
        MAP_ENTRY *hash_ptr = the_map.table[i];
        MAP_ENTRY *curr_map_entry = hash_ptr;
        MAP_ENTRY *next_map_entry = NULL;
        while( curr_map_entry != NULL){
            next_map_entry = curr_map_entry->next;
            clear_key_and_versions(curr_map_entry);
            curr_map_entry = next_map_entry;
        }
    }
}

/*
 * Put a key/value mapping in the store.  The key must not be NULL.
 * The value may be NULL, in which case this operation amounts to
 * deleting any existing mapping for the given key.
 *
 * This operation inherits the key and consumes one reference on
 * the value.
 *
 * @param tp  The transaction in which the operation is being performed.
 * @param key  The key.
 * @param value  The value.
 * @return  Updated status of the transation, either TRANS_PENDING,
 *   or TRANS_ABORTED.  The purpose is to be able to avoid doing further
 *   operations in an already aborted transaction.
 */

/*
typedef struct key {
    int hash;
    BLOB *blob;
} KEY;
*/

TRANS_STATUS insert_value_from_prev( TRANSACTION *tp , MAP_ENTRY *map_entry , BLOB **valuep){
    //BLOB *value = NULL;
    int my_id = tp -> id;
    VERSION *versions = map_entry->versions;

    if(  versions -> creator -> id  > my_id  ){

        return TRANS_ABORTED;
    }

    VERSION *curr_version = versions;
    VERSION *prev_version = NULL;
    while( curr_version != NULL ){
        if(curr_version -> creator -> id  < tp -> id ){
            prev_version = curr_version;
            curr_version = curr_version->next;

            continue;
        }
        if( curr_version -> creator -> id  ==  tp -> id  ){
            //clean_blob(curr_version -> blob );
            //curr_version -> blob = value;
            *valuep = curr_version -> blob;
            return tp -> status;
        }
        VERSION *new_version = version_create( tp , prev_version -> blob );
        new_version -> next = curr_version;
        new_version -> prev = prev_version;
        prev_version -> next = new_version;
        curr_version -> prev = new_version;
        add_dependencies(versions,tp);
        *valuep = new_version -> blob;
        return TRANS_PENDING;




    }
    VERSION *new_version = version_create(tp,prev_version -> blob);
    new_version->prev = prev_version;
    prev_version -> next = new_version;
    new_version -> next = NULL;
    add_dependencies(versions,tp);
    *valuep = new_version -> blob;
    return TRANS_PENDING;
}


KEY *get_new_copied_key(KEY *old_key){
    KEY *new_key = malloc(sizeof(KEY));
    new_key->hash = old_key->hash;
    BLOB *old_blob = old_key->blob;
    BLOB *new_blob = blob_create(old_blob->content, old_blob -> size);
    new_key->blob = new_blob;
    return new_key;
}

MAP_ENTRY *get_map_entry_from_search_space(MAP_ENTRY *search_space , KEY *target_key){
    if(search_space == NULL )return NULL;
    MAP_ENTRY *curr_map_entry = search_space;
    //MAP_ENTRY *next_map_entry = NULL;
    while( curr_map_entry  != NULL ){
        if( key_compare( curr_map_entry->key , target_key) == 0){
            return curr_map_entry;
        }
        curr_map_entry = curr_map_entry->next;
    }
    return NULL;
}

TRANS_STATUS store_put(TRANSACTION *tp, KEY *key, BLOB *value){
    if(tp -> status != TRANS_PENDING)
        return tp -> status;
    MAP_ENTRY *search_space = the_map.table[key->hash];
    MAP_ENTRY *map_entry = get_map_entry_from_search_space(search_space , key );

    if(value == NULL ){
        if(map_entry != NULL){
            VERSION *versions = map_entry -> versions;
            free_versions_and_abort(versions);
            versions -> creator -> status = TRANS_ABORTED;
            clean_blob( versions -> blob);
            free(versions);
            map_entry->versions = NULL;
            return tp->status;

        }
    }

    if( map_entry == NULL ){
        //TRANS_STATUS status;
        KEY *new_key = get_new_copied_key(key);
        MAP_ENTRY *new_map_entry = malloc(sizeof(MAP_ENTRY));
        new_map_entry->versions = version_create(tp,value);
        new_map_entry->key = new_key;
        //MAP_ENTRY *first = search_space->next;
        new_map_entry->next = search_space;
        the_map.table[new_key->hash] = new_map_entry;
        return tp->status;
        //create_new_key_value(tp,new_key,value,search_space);
    }else{
        TRANS_STATUS status;
        sortify_versions_and_garbage_collection(map_entry);
        status = insert_value(tp , map_entry , value);
        return status;
    }
}
/*
    VERSION *aborted_transaction = NULL;
    VERSION *committed_transaction = NULL;
    VERSION *pending_transaction = NULL;

*/

TRANS_STATUS insert_value( TRANSACTION *tp , MAP_ENTRY *map_entry , BLOB *value){
    VERSION *versions = map_entry->versions;
    if(versions == NULL) {
        VERSION *new_version = version_create(tp,value);
        map_entry->versions = new_version;
        add_dependencies(versions,tp);
        return TRANS_PENDING;
    }

    if(  versions -> creator -> id  > tp -> id  ){

        return TRANS_ABORTED;
    }

    VERSION *curr_version = versions;
    VERSION *prev_version = NULL;
    while( curr_version != NULL ){
        if(curr_version -> creator -> id  < tp -> id ){
            prev_version = curr_version;
            curr_version = curr_version->next;

            continue;
        }
        if( curr_version -> creator -> id  ==  tp -> id  ){
            clean_blob(curr_version -> blob );
            curr_version -> blob = value;
            return tp -> status;
        }
        VERSION *new_version = version_create(tp,value);
        new_version -> next = curr_version;
        new_version -> prev = curr_version -> prev;
        curr_version -> prev -> next = new_version;
        curr_version -> prev = new_version;
        add_dependencies(versions,tp);
        return TRANS_PENDING;




    }
    VERSION *new_version = version_create(tp,value);
    new_version->prev = prev_version;
    prev_version -> next = new_version;
    new_version -> next = NULL;
    add_dependencies(versions,tp);
    return TRANS_PENDING;
}

void add_dependencies(VERSION *ver , TRANSACTION *tp){
    while( ver -> creator -> id != tp -> id ){
        trans_add_dependency( tp, ver -> creator );
        ver = ver -> next;
    }
}

void sortify_versions_and_garbage_collection( MAP_ENTRY *map_entry ){
    VERSION *versions = map_entry->versions;
    if(versions  == NULL) return;
    //int aborted_transaction = -1;
    int committed_transaction = -1;






    VERSION *curr_version = versions;
    VERSION *prev_version = NULL;

    while( curr_version  != NULL ){

        if(curr_version -> creator -> status  == TRANS_ABORTED){

            if(prev_version == NULL){

                debug("smallest_transition_aborted");
                free_versions_and_abort(versions);
                versions -> creator -> status = TRANS_ABORTED;
                clean_blob( versions -> blob);
                free(versions);
                map_entry->versions = NULL;

            }else{
                free_versions_and_abort(prev_version);
                prev_version->next = NULL;
            }
            break;
        }

        if(curr_version -> creator -> status  == TRANS_COMMITTED )
            committed_transaction = curr_version -> creator -> id;

        prev_version = curr_version;
        curr_version = curr_version -> next;
    }
    if( committed_transaction == -1) return;

    versions = map_entry -> versions;
    if( versions  == NULL) return;

    curr_version = versions;
    VERSION *next_version = NULL;

    while (curr_version != NULL){
        if( curr_version -> creator -> id  == committed_transaction ){
            break;
        }
        clean_blob(curr_version -> blob);
        next_version = curr_version -> next;
        free(curr_version);
        curr_version = next_version;
    }
    map_entry->versions = curr_version;

}

void free_versions_and_abort(VERSION *version){
    VERSION *curr = version->next;
    VERSION *next = NULL;
    version->next = NULL;
    while( curr != NULL ){

        curr->creator->status = TRANS_ABORTED;
        clean_blob(curr->blob);
        next  = curr->next;
        free(curr);
        curr = next;

    }

}


/*
 * Get the value associated with a specified key.  A pointer to the
 * associated value is stored in the specified variable.
 *
 * This operation inherits the key.  The caller is responsible for
 * one reference on any returned value.
 *
 * @param tp  The transaction in which the operation is being performed.
 * @param key  The key.
 * @param valuep  A variable into which a returned value pointer may be
 *   stored.  The value pointer store may be NULL, indicating that there
 *   is no value currently associated in the store with the specified key.
 * @return  Updated status of the transation, either TRANS_PENDING,
 *   or TRANS_ABORTED.  The purpose is to be able to avoid doing further
 *   operations in an already aborted transaction.
 */
TRANS_STATUS store_get(TRANSACTION *tp, KEY *key, BLOB **valuep){
    if(tp -> status != TRANS_PENDING)
        return tp -> status;

    MAP_ENTRY *search_space = the_map.table[key->hash];
    MAP_ENTRY *map_entry = get_map_entry_from_search_space(search_space , key );


    if( map_entry == NULL ){
        *valuep = NULL;
        return tp -> status;
    }else{
        TRANS_STATUS status;
        sortify_versions_and_garbage_collection(map_entry);
        if(map_entry -> versions == NULL){
            *valuep = NULL;
            return tp -> status;
        }
        status = insert_value_from_prev(tp , map_entry , valuep);
        return status;
    }
}



/*
 * Print the contents of the store to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 */
void store_show(void);
