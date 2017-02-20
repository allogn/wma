//
// A class for working with igraph bipartite graphs
// Implements SIA and other matching algorithms
//

#ifndef NETWORKFLOW_BIPARTITE_H
#define NETWORKFLOW_BIPARTITE_H

#include <stack>
#include <algorithm>
#include <igraph/igraph.h>
#include <float.h>

class Bipartite {
public:
    igraph_t bigraph;
    igraph_vector_bool_t types; //types for bipartite graph
    igraph_vector_t edges;

    //random number generator
    igraph_rng_t rng;

    Bipartite() {
#ifndef IGR_CATTRIBUTE_ON
#define IGR_CATTRIBUTE_ON
        //turn on error igraph attribute handling globally
        igraph_i_set_attribute_table(&igraph_cattribute_table);
#endif

        igraph_vector_bool_init(&types,2);
        igraph_vector_init(&edges,2);

        //initialize with a simpliest graph because empty graph is not supported by bipartite igraph
        VECTOR(types)[0] = false;
        VECTOR(types)[1] = true;
        VECTOR(edges)[0] = 1;
        VECTOR(edges)[1] = 0;
        igraph_create_bipartite(&bigraph,&types,&edges,true); //create DIRECTED bipartite graph

        //init random generator
        igraph_rng_init(&rng, &igraph_rngtype_mt19937);
        unsigned long seed = static_cast<unsigned long>(time(0));
        igraph_rng_seed(&rng, seed);
    }

    ~Bipartite() {
        igraph_destroy(&bigraph);
        igraph_rng_destroy(&rng);
    }

    /*
     * Matching algorithms
     */
    // calculate maximum matching using push-relabel algorithm
    int igraph_max_matching(igraph_real_t* result_weight, igraph_vector_long_t* result_matching) {
        igraph_integer_t  matching_size; //will be NULL

        //check whether graph is weighted
        if (!igraph_cattribute_has_attr(&bigraph, IGRAPH_ATTRIBUTE_EDGE, "weight")) {
            throw std::string("Weights of bipartite graph must be initialized before matching");
        }
        igraph_vector_t weights;
        igraph_vector_init(&weights,0);
        EANV(&bigraph, "weight", &weights);

        return igraph_maximum_bipartite_matching(&bigraph, &types, &matching_size, result_weight,
                                          result_matching, &weights, DBL_EPSILON); //DBL_EPSILON is float number error
    }

    //calculates maximum matching using SIA algorithm
    int sia_max_matching(igraph_real_t* result_weight, igraph_vector_long_t* result_matching);

    /*
     * Generators
     */
    // generates full bipartite graph. Current graph with vertices are ignored
    // input: n1 and n2 are number of vertices of two types (<n1> --> <n2>)
    int generate_full_bipartite(igraph_integer_t n1, igraph_integer_t n2) {
        //returns error code
        return igraph_full_bipartite(&bigraph, &types, n1, n2, true, IGRAPH_OUT);
    }

    // generates random weights for currently initialized bipartite graph
    void generate_random_weights() {
        igraph_vector_t weights;
        igraph_integer_t esize = igraph_ecount(&bigraph);
        igraph_vector_init(&weights, esize);
        for (igraph_integer_t i = 0; i < esize; i++) {
            igraph_vector_set(&weights, i, igraph_rng_get_unif01(&rng));
        }
        igraph_cattribute_EAN_setv(&bigraph, "weight", &weights);
    }

};

#endif //NETWORKFLOW_BIPARTITE_H
