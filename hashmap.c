#include "hashmap.h"
#include "qalloc.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

void hashmap_init( struct hashmap* map,
                   size_t ( *hash )( struct hashmap*, void* ),
                   void ( *key_del )( void* ), void ( *val_del )( void* ),
                   int ( *cmp )( void*, void* ), size_t size, size_t objsize ) {
    map->hash    = hash;
    map->key_del = key_del;
    map->val_del = val_del;
    map->cmp     = cmp;

    map->size    = size;
    map->keysize = objsize;
    map->buckets =
        (struct bucket*) mem_calloc( map->size, sizeof( struct bucket ) );
    map->count = 0;
    pthread_mutex_init( &map->count_mut, NULL );
    map->mutexs =
        (pthread_mutex_t*) mem_calloc( map->size, sizeof( pthread_mutex_t ) );
    for ( int i = 0; i < map->size; i++ ) {
        pthread_mutex_init( map->mutexs + i, NULL );
    }
}

void hashmap_insert( struct hashmap* map, void* key, void* value ) {

    size_t index = map->hash( map, key ) % map->size;
    pthread_mutex_lock( map->mutexs + index );
    struct bucket* bucket = map->buckets + index;
    struct bucket* p      = bucket->next;
    while ( p != NULL ) {
        if ( map->cmp( p->key, key )==0 ) {
            break;
        }
        p=p->next;
    }
    if ( p == NULL ) {
        p            = (struct bucket*) mem_alloc(sizeof(struct bucket));
        p->next      = bucket->next;
        bucket->next = p;
        pthread_mutex_lock(&map->count_mut);
        map->count += 1;
        pthread_mutex_unlock(&map->count_mut);
    }
    p->key   = key;
    p->value = value;
    pthread_mutex_unlock( map->mutexs + index );
}

void* hashmap_get( struct hashmap* map, void* key ) {
    size_t index = map->hash( map, key ) % map->size;
    pthread_mutex_lock( map->mutexs + index );
    struct bucket* bucket = map->buckets + index;
    struct bucket* p      = bucket->next;
    while ( p != NULL ) {
        if ( map->cmp( p->key, key )==0 ) {
            break;
        }
        p = p->next;
    }
    void* value;
    if ( p != NULL )
        value = p->value;
    else
        value = NULL;
    pthread_mutex_unlock( map->mutexs + index );
    return value;
}

void* hashmap_get_entry( struct hashmap* map, void* key ) {

    size_t index = map->hash( map, key ) % map->size;
    pthread_mutex_lock( map->mutexs + index );
    struct bucket* bucket = map->buckets + index;
    struct bucket* p      = bucket->next;
    while ( p != NULL ) {
        if ( map->cmp( p->key, key )==0 ) {
            break;
        }
        p = p->next;
    }
    void* entry = p;
    pthread_mutex_unlock( map->mutexs + index );
    return entry;
}

void* hashmap_remove_value( struct hashmap* map, void* key ) {
    size_t index = map->hash( map, key ) % map->size;
    pthread_mutex_lock( map->mutexs + index );
    struct bucket* bucket = map->buckets + index;
    struct bucket* sp     = bucket;
    struct bucket* p      = bucket->next;
    while ( p != NULL ) {
        if ( map->cmp( p->key, key )==0 ) {
            break;
        }
        sp = p;
        p  = p->next;
    }
    void* value;
    if ( p != NULL ) {
        pthread_mutex_lock(&map->count_mut);
        map->count -= 1;
        pthread_mutex_unlock(&map->count_mut);
        value    = p->value;
        sp->next = p->next;
        map->key_del(p->key);
        mem_free( p );
    } else {
        value = NULL;
    }
    pthread_mutex_unlock( map->mutexs + index );
    return value;
}

void* hashmap_remove_entry( struct hashmap* map, void* key ) {
    size_t index = map->hash( map, key ) % map->size;
    pthread_mutex_lock( map->mutexs + index );
    struct bucket* bucket = map->buckets + index;
    struct bucket* sp     = bucket;
    struct bucket* p      = bucket->next;
    while ( p != NULL ) {
        if ( map->cmp( p->key, key )==0 ) {
            break;
        }
        sp = p;
        p  = p->next;
    }
    struct bucket* entry;
    if ( p != NULL ) {
        pthread_mutex_lock(&map->count_mut);
        map->count -= 1;
        pthread_mutex_unlock(&map->count_mut);
 
        sp->next = p->next;
        entry = (struct bucket*)mem_alloc(sizeof(struct bucket));
        memcpy(entry,p,sizeof(struct bucket));
        entry->next = NULL;
        mem_free( p );
    } else {
        entry = NULL;
    }
    pthread_mutex_unlock( map->mutexs + index );
    return entry;
}

size_t hashmap_size( struct hashmap* map ) {
    pthread_mutex_lock(&map->count_mut);
    size_t count = map->count;
    pthread_mutex_unlock(&map->count_mut);
    return count;
}

void hashmap_destroy( struct hashmap* map ) {
    pthread_mutex_lock(&map->count_mut);
    pthread_mutex_destroy(&map->count_mut);
    for ( int i = 0; i < map->size; i++ ) {
        pthread_mutex_lock(map->mutexs+i);
        struct bucket* p = (map->buckets + i)->next;
        while(p!=NULL){
            map->key_del(p->key);
            map->val_del(p->value);
            mem_free(p);
            p=p->next;
        }
        pthread_mutex_destroy(map->mutexs+i);
    }
    mem_free( map->buckets );
    mem_free( map->mutexs );
}
