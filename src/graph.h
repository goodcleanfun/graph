#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#ifndef GRAPH_NAME
#error "Must define GRAPH_NAME"
#endif

// Most in-memorygraphs will only need 32 bit indices
#ifndef GRAPH_INDEX_TYPE
#define GRAPH_INDEX_TYPE uint32_t
#endif

#ifndef GRAPH_EDGE_TYPE
#define GRAPH_EDGE_TYPE uint32_t
#endif

#ifndef GRAPH_MALLOC
#define GRAPH_MALLOC malloc
#define GRAPH_MALLOC_DEFINED
#endif

#ifndef GRAPH_FREE
#define GRAPH_FREE free
#define GRAPH_FREE_DEFINED
#endif

#ifndef GRAPH_DEFAULT_NUM_NODES
#define GRAPH_DEFAULT_NUM_NODES 128
#endif

#ifndef GRAPH_DEFAULT_NUM_EDGES
#define GRAPH_DEFAULT_NUM_EDGES 2
#endif

#define GRAPH_CONCAT_(a, b) a ## b
#define GRAPH_CONCAT(a, b) GRAPH_CONCAT_(a, b)
#define GRAPH_TYPED(name) GRAPH_CONCAT(GRAPH_NAME, _##name)
#define GRAPH_FUNC(name) GRAPH_CONCAT(GRAPH_NAME, _##name)

#ifndef EDGE_ARRAY_ALIGNMENT
#ifdef __AVX512F__
#define EDGE_ARRAY_ALIGNMENT 64
#elif defined(__AVX__)
#define EDGE_ARRAY_ALIGNMENT 32
#elif defined(__SSE__)
#define EDGE_ARRAY_ALIGNMENT 16
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#define EDGE_ARRAY_ALIGNMENT 16
#else
#define EDGE_ARRAY_ALIGNMENT 16
#endif
#define EDGE_ARRAY_ALIGNMENT_DEFINED
#endif

#ifndef EDGE_ARRAY_MALLOC
#define EDGE_ARRAY_MALLOC(size) aligned_malloc(size, EDGE_ARRAY_ALIGNMENT)
#define EDGE_ARRAY_MALLOC_DEFINED
#endif

#ifndef EDGE_ARRAY_REALLOC
#define EDGE_ARRAY_REALLOC(a, prev_size, new_size) aligned_resize(a, prev_size, new_size, EDGE_ARRAY_ALIGNMENT)
#define EDGE_ARRAY_REALLOC_DEFINED
#endif

#ifndef EDGE_ARRAY_FREE
#define EDGE_ARRAY_FREE(a) default_aligned_free(a)
#define EDGE_ARRAY_FREE_DEFINED
#endif

#define ARRAY_NAME GRAPH_TYPED(edge_array)
#define ARRAY_TYPE GRAPH_EDGE_TYPE

#define ARRAY_ALIGNMENT EDGE_ARRAY_ALIGNMENT
#define ARRAY_MALLOC EDGE_ARRAY_MALLOC
#define ARRAY_REALLOC EDGE_ARRAY_REALLOC
#define ARRAY_FREE EDGE_ARRAY_FREE

#include "aligned_array/aligned_array.h"

#undef ARRAY_ALIGNMENT
#undef ARRAY_MALLOC
#undef ARRAY_REALLOC
#undef ARRAY_FREE

#undef ARRAY_NAME
#undef ARRAY_TYPE


#define GRAPH_EDGE_ARRAY GRAPH_TYPED(edge_array)
#define GRAPH_EDGE_ARRAY_FUNC(name) GRAPH_CONCAT(GRAPH_EDGE_ARRAY, _##name)


typedef struct GRAPH_TYPED(node) {
    GRAPH_INDEX_TYPE id;
    // The array struct is not a pointer
    GRAPH_EDGE_ARRAY edges;
} GRAPH_TYPED(node_t);

#define GRAPH_NODE_TYPE GRAPH_TYPED(node_t)

#define ARRAY_NAME GRAPH_TYPED(node_array)
#define ARRAY_TYPE GRAPH_NODE_TYPE

#include "aligned_array/aligned_array.h"

#undef ARRAY_NAME
#undef ARRAY_TYPE

#define GRAPH_NODE_ARRAY GRAPH_TYPED(node_array)
#define GRAPH_NODE_ARRAY_FUNC(name) GRAPH_CONCAT(GRAPH_NODE_ARRAY, _##name)

typedef struct {
    GRAPH_NODE_ARRAY *nodes;
} GRAPH_NAME;

GRAPH_NAME *GRAPH_FUNC(new_num_nodes)(size_t num_nodes) {
    GRAPH_NAME *g = GRAPH_MALLOC(sizeof(GRAPH_NAME));
    if (g == NULL) {
        return NULL;
    }

    g->nodes = GRAPH_NODE_ARRAY_FUNC(new_size)(num_nodes);
    if (g->nodes == NULL) {
        free(g);
        return NULL;
    }

    return g;
}

GRAPH_NAME *GRAPH_FUNC(new)(void) {
    return GRAPH_FUNC(new_num_nodes)(GRAPH_DEFAULT_NUM_NODES);
}


static bool GRAPH_FUNC(add_node)(GRAPH_NAME *g, GRAPH_EDGE_TYPE *id) {
    if (g == NULL || g->nodes == NULL || id == NULL) return false;
    GRAPH_NODE_TYPE node;
    node.edges.a = EDGE_ARRAY_MALLOC(GRAPH_DEFAULT_NUM_EDGES * sizeof(GRAPH_EDGE_TYPE));
    if (node.edges.a == NULL) return false;
    node.edges.m = GRAPH_DEFAULT_NUM_EDGES;
    node.edges.n = 0;
    size_t index;
    index = GRAPH_NODE_ARRAY_FUNC(size)(g->nodes);
    node.id = (GRAPH_EDGE_TYPE)index;
    if (!GRAPH_NODE_ARRAY_FUNC(push)(g->nodes, node)) return false;

    if (id != NULL) {
        *id = node.id;
    }
    return true;
}

static inline bool GRAPH_FUNC(add_edge)(GRAPH_NAME *g, GRAPH_NODE_TYPE *node, GRAPH_EDGE_TYPE edge) {
    if (g == NULL || g->nodes == NULL) return false;
    return GRAPH_EDGE_ARRAY_FUNC(push)(&node->edges, edge);
}

static inline bool GRAPH_FUNC(get_node)(GRAPH_NAME *g, GRAPH_INDEX_TYPE id, GRAPH_NODE_TYPE **node) {
    if (g == NULL || g->nodes == NULL) return false;
    if (id >= GRAPH_NODE_ARRAY_FUNC(size)(g->nodes)) return false;

    *node = g->nodes->a + id;
    return true;
}

static inline void GRAPH_FUNC(clear)(GRAPH_NAME *g) {
    if (g == NULL || g->nodes == NULL) return;
    size_t n = GRAPH_NODE_ARRAY_FUNC(size)(g->nodes);
    GRAPH_NODE_TYPE node;
    for (size_t i = 0; i < n; i++) {
        if (!GRAPH_NODE_ARRAY_FUNC(get)(g->nodes, i, &node)) continue;
        GRAPH_EDGE_ARRAY_FUNC(clear)(&node.edges);
    }
    GRAPH_NODE_ARRAY_FUNC(clear)(g->nodes);
}

static inline void GRAPH_FUNC(destroy)(GRAPH_NAME *g) {
    if (g == NULL) return;
    if (g->nodes != NULL) {
        size_t n = GRAPH_NODE_ARRAY_FUNC(size)(g->nodes);
        GRAPH_NODE_TYPE *node;
        for (size_t i = 0; i < n; i++) {
            if (!GRAPH_FUNC(get_node)(g, i, &node)) continue;
            EDGE_ARRAY_FREE(node->edges.a);
        }
        GRAPH_NODE_ARRAY_FUNC(destroy)(g->nodes);
    }
    GRAPH_FREE(g);
}


#ifdef GRAPH_MALLOC_DEFINED
#undef GRAPH_MALLOC
#undef GRAPH_MALLOC_DEFINED
#endif

#ifdef GRAPH_FREE_DEFINED
#undef GRAPH_FREE
#undef GRAPH_FREE_DEFINED
#endif

#ifdef GRAPH_MALLOC_DEFINED
#undef GRAPH_MALLOC
#undef GRAPH_MALLOC_DEFINED
#endif

#ifdef EDGE_ARRAY_ALIGNMENT_DEFINED
#undef EDGE_ARRAY_ALIGNMENT
#undef EDGE_ARRAY_ALIGNMENT_DEFINED
#endif

#ifdef EDGE_ARRAY_MALLOC_DEFINED
#undef EDGE_ARRAY_MALLOC
#undef EDGE_ARRAY_MALLOC_DEFINED
#endif

#ifdef EDGE_ARRAY_REALLOC_DEFINED
#undef EDGE_ARRAY_REALLOC
#undef EDGE_ARRAY_REALLOC_DEFINED
#endif

#ifdef EDGE_ARRAY_FREE_DEFINED
#undef EDGE_ARRAY_FREE
#undef EDGE_ARRAY_FREE_DEFINED
#endif