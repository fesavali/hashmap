#include "hashmap.h"
#include "qalloc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define TEST_LEN 100000
#define TEST_KEY_LEN 10

clock_t    start, finish;
static int str_cmp( void* a, void* b ) {
    return strcmp( (char*) a, (char*) b );
}
static void key_del( void* k ) { return; };
static void val_del( void* k ) { return; };

// djb2, simple
static size_t djb2_hash( struct hashmap* map, void* k ) {
    size_t sz   = map->keysize; // replace this with the key size
    char*  p    = k;
    size_t hash = 5381;
    for ( int i = 0; i < sz; i++ ) {
        hash = ( ( hash << 5 ) + hash ) + p[ i ];
    }
    return hash;
}

void hash_test( char keys[][ TEST_KEY_LEN + 1 ],
                char vals[][ TEST_KEY_LEN + 1 ], int keys_size ) {
    struct hashmap map;
    hashmap_init( &map, djb2_hash, key_del, val_del, str_cmp, 10000,
                  TEST_KEY_LEN );
    start = clock();
    for ( int i = 0; i < keys_size; i++ ) {
        hashmap_insert( &map, keys[ i ], vals[ i ] );
    }
    finish = clock();
    printf( "hash insert %d total time:%f\n", keys_size,
            (double) ( finish - start ) / CLOCKS_PER_SEC );
    start      = clock();
    int sucess = 0;
    for ( int i = 0; i < keys_size; i++ ) {
        char* val = hashmap_get( &map, keys[ i ] );
        if ( strcmp( val, vals[ i ] ) == 0 ) {
            sucess += 1;
        }
    }
    finish = clock();
    printf( "hash find %d(%d) total time:%f\n", keys_size, sucess,
            (double) ( finish - start ) / CLOCKS_PER_SEC );
    hashmap_destroy( &map );
}
struct link {
    void* key;
    void* val;
};

void link_test( char keys[][ TEST_KEY_LEN + 1 ],
                char vals[][ TEST_KEY_LEN + 1 ], int keys_size ) {
    struct link* link =
        (struct link*) mem_alloc( keys_size * sizeof( struct link ) );
    start = clock();
    for ( int i = 0; i < keys_size; i++ ) {
        link[ i ].key = keys[ i ];
        link[ i ].val = vals[ i ];
    }
    finish = clock();
    printf( "link insert %d total time:%f\n", keys_size,
            (double) ( finish - start ) / CLOCKS_PER_SEC );
    start      = clock();
    int sucess = 0;
    for ( int i = 0; i < keys_size; i++ ) {
        char* val;
        for ( int j = 0; j < keys_size; j++ ) {
            if ( link[ i ].key == keys[ i ] ) {
                val = link[ i ].val;
            }
        }
        if ( strcmp( val, vals[ i ] ) == 0 ) {
            sucess += 1;
        }
    }
    finish = clock();
    printf( "link find %d(%d) total time:%f\n", keys_size, sucess,
            (double) ( finish - start ) / CLOCKS_PER_SEC );
    mem_free( link );
}
int main() {
    int  keys_size = TEST_LEN;
    char keys[ TEST_LEN ][ TEST_KEY_LEN + 1 ];
    char vals[ TEST_LEN ][ TEST_KEY_LEN + 1 ];
    char kkey[ TEST_KEY_LEN + 1 ] = { 'A' };
    for ( int i = 0; i < TEST_KEY_LEN; i++ ) {
        kkey[ i ] = 'A';
    }
    kkey[ TEST_KEY_LEN ] = '\0';
    for ( int i = 0; i < TEST_LEN; i++ ) {
        strcpy( keys[ i ], kkey );

        for ( int j = 0; j < TEST_KEY_LEN; j++ ) {
            vals[ i ][ j ] = rand() % 26 + 'A';
        }
        vals[ i ][ TEST_KEY_LEN ] = '\0';
        for ( int j = 0; j < TEST_KEY_LEN; j++ ) {
            kkey[ j ] += 1;
            if ( kkey[ j ] > 'Z' ) {
                kkey[ j ] = 'A';
            } else {
                break;
            }
        }
    }

    hash_test( keys, vals, keys_size );
    link_test( keys, vals, keys_size );
}
