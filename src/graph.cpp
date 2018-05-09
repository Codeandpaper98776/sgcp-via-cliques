//
// Created by alberto on 08/05/18.
//

#include "graph.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <as/and_die.h>
#include <as/repeat.h>
#include <as/graph.h>
#include <as/containers.h>

namespace sgcp_cliques {
    namespace {
        void add_partition_cliques(ClusteredGraph& graph) {
            for(auto v = 0u; v < boost::num_vertices(graph); ++v) {
                for(auto w = v + 1; w < boost::num_vertices(graph); ++w) {
                    if(graph[v] == graph[w]) {
                        if(!boost::edge(v, w, graph).second) {
                            boost::add_edge(v, w, graph);
                        }
                    }
                }
            }
        }

        bool is_simplicial_pair(std::size_t v1, std::size_t v2, std::size_t w1, std::size_t w2, const DirectedGraph& dgraph) {
            const auto [e, e_present] = boost::edge(
                std::min(v1, v2), std::max(v1, v2), dgraph
            );

            const auto [f, f_present] = boost::edge(
                std::min(w1, w2), std::max(w1, w2), dgraph
            );

            (void) e_present, (void) f_present;
            assert(e_present && f_present);

            if(boost::source(e, dgraph) == boost::source(f, dgraph)) {
                const auto et = boost::target(e, dgraph);
                const auto ft = boost::target(f, dgraph);

                if(boost::edge(et, ft, dgraph).second || boost::edge(ft, et, dgraph).second) {
                    return true;
                }
            }

            return false;
        }
    }

    ClusteredGraph read_clustered_graph(std::string graph_file) {
        using as::and_die;

        std::ifstream ifs(graph_file);

        if(ifs.fail()) {
            std::cerr << "Cannot read file " << graph_file << and_die();
            std::exit(1);
        }

        std::size_t num_vertices, num_edges, num_partitions;

        if(!(ifs >> num_vertices)) {
            std::cerr << "Cannot read number of vertices from " << graph_file << and_die();
        }

        if(!(ifs >> num_edges)) {
            std::cerr << "Cannot read number of edges from " << graph_file << and_die();
        }

        if(!(ifs >> num_partitions)) {
            std::cerr << "Cannot read number of partitions from " << graph_file << and_die();
        }

        ClusteredGraph graph;
        graph[boost::graph_bundle] = num_partitions;

        as::repeat(num_vertices, [&] () { boost::add_vertex(graph); });

        as::repeat(num_edges, [&] () {
            std::size_t source, target;

            if(!(ifs >> source >> target)) {
                std::cerr << "Cannot read an edge from " << graph_file << and_die();
            }

            boost::add_edge(source, target, graph);
        });

        std::stringstream ss;
        std::string line;

        ifs >> std::ws;

        for(auto partition = 0u; partition < num_partitions; ++partition) {
            std::getline(ifs, line);
            ss.str(line);

            std::size_t vertex;

            while(ss >> vertex) {
                graph[vertex] = partition;
            }

            ss.str("");
            ss.clear();
        }

        add_partition_cliques(graph);

        return graph;
    }

    LineGraph line_graph(const ClusteredGraph &cgraph) {
        LineGraph lgraph(boost::num_edges(cgraph));

        std::size_t lvertex = 0u;
        for(const auto& edge : as::graph::edges(cgraph)) {
            lgraph[lvertex++] = std::make_pair(
                boost::target(edge, cgraph),
                boost::source(edge, cgraph)
            );
        }

        for(auto e = 0u; e < boost::num_vertices(lgraph); ++e) {
            const auto cl_e1 = cgraph[lgraph[e].first];
            const auto cl_e2 = cgraph[lgraph[e].second];

            for(auto f = e + 1; f < boost::num_vertices(lgraph); ++f) {
                const auto cl_f1 = cgraph[lgraph[f].first];
                const auto cl_f2 = cgraph[lgraph[f].second];

                if(cl_e1 == cl_f1 || cl_e1 == cl_f2 || cl_e2 == cl_f1 || cl_e2 == cl_f2) {
                    boost::add_edge(e, f, lgraph);
                }
            }
        }

        return lgraph;
    }

    DirectedGraph directed_acyclic(const ClusteredGraph &cgraph) {
        return as::graph::acyclic_orientation(cgraph);
    }

    LineGraph sandwich_line_graph(const ClusteredGraph& cgraph) {
        const auto lgraph = line_graph(cgraph);
        const auto dgraph = directed_acyclic(cgraph);
        LineGraph slgraph(boost::num_vertices(lgraph));

        for(auto e = 0u; e < boost::num_vertices(slgraph); ++e) {
            slgraph[e] = lgraph[e];

            const auto e_vertex_1 = lgraph[e].first;
            const auto e_vertex_2 = lgraph[e].second;

            for(auto f = e + 1; f < boost::num_vertices(slgraph); ++f) {
                const auto f_vertex_1 = lgraph[f].first;
                const auto f_vertex_2 = lgraph[f].second;

                if( boost::edge(e, f, lgraph).second &&
                    ! is_simplicial_pair(e_vertex_1, e_vertex_2, f_vertex_1, f_vertex_2, dgraph)
                ) {
                    boost::add_edge(e, f, slgraph);
                }
            }
        }

        return slgraph;
    }

    LineGraph complementary_sandwich_line_graph(const ClusteredGraph &cgraph) {
        const auto slgraph = sandwich_line_graph(cgraph);
        return as::graph::complementary(slgraph);
    }
}