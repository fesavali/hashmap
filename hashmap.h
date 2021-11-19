#ifndef _HASHMAP_H_
#define _HASHMAP_H_
#include <stddef.h>
#include <pthread.h>

#define HASHMAP_DEFAULT_CAPACITY 20
#define HASHMAP_MAX_LOAD 0.75f
#define HASHMAP_RESIZE_FACTOR 2

struct bucket
{
	void* key;
    void* value;
    struct bucket *next;
};


/*
 * Type symbol for the hashmap, implementation
 * is defined by the student but must represent a hashmap
 */
struct hashmap {
    size_t ( *hash )( struct hashmap*, void* );
    void ( *key_del )( void* );
    void ( *val_del )( void* );
    int ( *cmp )( void*, void* );
    size_t size;
    size_t keysize;

    struct bucket* buckets;
    pthread_mutex_t *mutexs;	// mut for each bucket
    pthread_mutex_t count_mut;	// mut for count
    size_t count;
};

/**
 * Initialises your data structure with the following
 * arguments.
 */
void hashmap_init(struct hashmap* map, 
		size_t(*hash)(struct hashmap*,void*), 
		void(*key_del)(void*),
		void(*val_del)(void*), 
		int(*cmp)(void*, void*),
		size_t size, size_t keysize);

/**
 * Inserts a key and value 
 */
void hashmap_insert(struct hashmap*, 
		void* key, 
		void* value);

void* hashmap_get(struct hashmap* map, 
		void* key);

void* hashmap_get_entry(struct hashmap* map, 
		void* key);

void* hashmap_remove_value(struct hashmap* map, 
		void* key);

void* hashmap_remove_entry(struct hashmap* map, 
		void* key);

size_t hashmap_size(struct hashmap* map);

void hashmap_destroy(struct hashmap* map);
#endif
