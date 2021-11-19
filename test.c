#include "hashmap.h"
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

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

static int str_cmp( void* a, void* b ) {
    return strcmp( (char*) a, (char*) b );
}
static void key_del( void* k ) { return; };
static void val_del( void* k ) { return; };

// base test
static void test_hashmap_base( void** state ) {
    struct hashmap map;
    hashmap_init( &map, djb2_hash, key_del, val_del, str_cmp, 3, 10 );
    char  key1[] = "abc";
    char  key2[] = "aac";
    char  key3[] = "adc";
    char  key4[] = "bbc";
    char* s;
    hashmap_insert( &map, key1, key2 );
    s = (char*) hashmap_get( &map, key1 ); // in hashmap
    assert_ptr_equal( s, key2 );
    s = (char*) hashmap_get( &map, key2 ); // not in hashmap
    assert_ptr_equal( s, NULL );
    s = (char*) hashmap_remove_value( &map, key2 );
    assert_ptr_equal( s, NULL );
    s = (char*) hashmap_remove_value( &map, key1 );
    assert_ptr_equal( s, key2 );

    hashmap_insert( &map, key1, key2 );
    struct bucket* b;
    b = (struct bucket*) hashmap_get_entry( &map, key1 );
    assert_ptr_equal( b->key, key1 );
    assert_ptr_equal( b->value, key2 );
    b = (struct bucket*) hashmap_get_entry( &map, key2 );
    assert_ptr_equal( b, NULL );
    b = (struct bucket*) hashmap_remove_entry( &map, key1 );
    assert_ptr_equal( b->key, key1 );
    assert_ptr_equal( b->value, key2 );
    b = (struct bucket*) hashmap_get_entry( &map, key1 );
    assert_ptr_equal( b, NULL );
}

static void key1_del( void* k ) { free( k ); }
static void val1_del( void* k ) { free( k ); }

static void* work_insert( void* _map ) {
    struct hashmap* map = _map;
    for ( int i = 0; i < 1000; i++ ) {
        char* k = (char*) malloc( 5 * sizeof( char ) );
        char* v = (char*) malloc( 5 * sizeof( char ) );
        for ( int j = 0; j < 4; j++ ) {
            k[ j ] = rand() %10+'A';
            v[ j ] = k[ j ];
        }
        hashmap_insert( map, k, v );
    }
    return NULL;
}
static void* work_read( void* _map ) {
    struct hashmap* map = _map;
    for ( int i = 0; i < 1000; i++ ) {
        char *k, *v;
        k = (char*) malloc( 5 * sizeof( char ) );
        for ( int j = 0; j < 4; j++ ) {
            k[ j ] = rand() %10+'A';
        }
        v = hashmap_get( map, k );
        if ( v != NULL ) {
            assert_string_equal( v, k );
        }
    }
    return NULL;
}

// concurrent
static void test_concurrent( void** state ) {
    struct hashmap map;
    hashmap_init( &map, djb2_hash, key1_del, val1_del, str_cmp, 4, 10 );
    pthread_t tids[ 10 ];
    for ( int i = 0; i < 3; i++ ) {
        if ( i % 2 == 0 ) {
            pthread_create( tids + i, NULL, work_read, &map );
        } else {
            pthread_create( tids + i, NULL, work_insert, &map );
        }
    }
    for ( int i = 0; i < 3; i++ ) {
        pthread_join( tids[ i ], NULL );
    }
}

// 

int main() {
    test_concurrent( NULL );
    const struct CMUnitTest tests[] = { cmocka_unit_test( test_hashmap_base ),
                                        cmocka_unit_test( test_concurrent ) };
    return cmocka_run_group_tests( tests, NULL, NULL );
}
