//
// Created by alberto on 01/10/18.
//

#ifndef SGCP_CLIQUES_GRAPH_WEIGHTED_H
#define SGCP_CLIQUES_GRAPH_WEIGHTED_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <cstddef>
#include <vector>

namespace smwgcp_cliques {
  // Properties of the clustered graph.
  struct ClusteredGraphProperties {
    // Number of clusters in the graph.
    std::size_t num_clusters;

    // Actual clusters of the graph.
    std::vector<std::vector<std::size_t>> clusters;
  };

  // Properties of the vertices of the clustered graph.
  struct ClusteredVertexProperties {
    // The vertex belongs to this cluster.
    std::size_t cluster;

    // The vertex has this weight.
    float weight;

    explicit ClusteredVertexProperties(float weight) : cluster{0u}, weight{weight} {}
    ClusteredVertexProperties() : cluster{0u}, weight{0.0f} {}
  };

  // Underlying graph of the Selective Maximum Weight Graph Colouring Problem.
  using ClusteredWeightedGraph = boost::adjacency_list<
      boost::vecS, boost::vecS, boost::undirectedS,
      ClusteredVertexProperties,
      boost::no_property,
      ClusteredGraphProperties
  >;

  // Line graph of the original graph.
  // It has a vertex for each edge of the original graph.
  // It has an edge between two vertices if the corresponding edges in the
  // original graph have one endpoint in a common cluster.
  using LineGraph = boost::adjacency_list<
      boost::vecS, boost::vecS, boost::undirectedS,
      std::pair<std::size_t, std::size_t>
  >;

  // A sandwich line graph imposes the additional requirement that
  // the edges in the original graph do not form a simplicial pair.
  // Only in this case, the corresponding vertices in the line graph
  // will be linked by an edge.
  using SandwichLineGraph = LineGraph;

  // An acyclic orientation of the ClusteredWeightedGraph.
  using DirectedGraph = boost::adjacency_list<
      boost::vecS, boost::vecS, boost::directedS,
      ClusteredVertexProperties,
      boost::no_property,
      ClusteredGraphProperties
  >;

  // Return the number of partitions, which is stored as a graph property.
  inline std::size_t number_of_partitions(const ClusteredWeightedGraph& cwgraph) { return cwgraph[boost::graph_bundle].num_clusters; }

  // Reads a clustered weighted graph from file.
  // First line: the number N of vertices;
  // Second line: the number M of edges;
  // Third line: the number P of clusters;
  // Next N lines: the weights of the vertices;
  // Next M lines: one edge per line, vertices are separated by one whitespace and numbered from 0 to N-1;
  // Nest P lines: one cluster per line, same as above.
  ClusteredWeightedGraph read_clustered_weighted_graph(std::string graph_file);

    // Produces an acyclic orientation of the original graph, based on vertex weights.
    DirectedGraph directed_acyclic(const ClusteredWeightedGraph& cwgraph);

    // Creates the line graph from the original graph.
    // See the description above the definition of LineGraph.
    LineGraph line_graph(const ClusteredWeightedGraph& cwgraph);

    // Creates the sandwich line graph from the original graph.
    // See the description above the definition of LineGraph.
    LineGraph sandwich_line_graph(const ClusteredWeightedGraph& cwgraph);

    // Creates the complementary of the sandwich line graph.
    // This is done to solve a max-clique problem instead of a
    // max-stable-set one.
    LineGraph complementary_sandwich_line_graph(const ClusteredWeightedGraph& cwgraph);
}

#endif //SGCP_CLIQUES_GRAPH_WEIGHTED_H
