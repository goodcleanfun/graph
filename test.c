#include <stdint.h>
#include "greatest/greatest.h"
#include "char_array/char_array.h"

#define GRAPH_NAME test_graph
#define GRAPH_INDEX_TYPE uint32_t
#define GRAPH_EDGE_TYPE uint32_t
#include "graph.h"
#undef GRAPH_NAME
#undef GRAPH_INDEX_TYPE
#undef GRAPH_EDGE_TYPE

TEST test_graph_construction(void) {
    test_graph *g = test_graph_new();

    uint32_t id;
    test_graph_node_t *node;
    size_t num_nodes = 1000;
    size_t num_edges_per_node = 50;

    for (size_t i = 0; i < num_nodes; i++) {
        test_graph_add_node(g, &id);
        ASSERT_EQ(id, i);

        ASSERT(test_graph_get_node(g, id, &node));
        ASSERT_EQ(node->id, id);

        for (size_t j = 1; j <= num_edges_per_node; j ++) {
            test_graph_add_edge(g, node, (i + j) % num_nodes);
        }

        ASSERT(test_graph_get_node(g, id, &node));
        ASSERT_EQ(node->edges.n, num_edges_per_node);
        for (size_t j = 0; j < num_edges_per_node; j++) {
            ASSERT_EQ(node->edges.a[j], (i + j + 1) % num_nodes);
        }
    }

    test_graph_destroy(g);
    PASS();
}


/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int32_t main(int32_t argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_graph_construction);

    GREATEST_MAIN_END();        /* display results */
}
