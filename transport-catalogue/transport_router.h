#pragma once
#include "graph.h"
#include "domain.h"
#include "transport_catalogue.h"

class RouteGraph {
public:
	RouteGraph(const TransportGuide::TransportCatalogue& tc_) : tc(tc_) {}

	void GraphAddRouteEdges(graph::DirectedWeightedGraph<domain::GraphEdge>& graph_, const std::vector<std::string_view>& stops_,
		std::string_view bus_, const size_t i_start, const size_t i_stop);
	void BuildGraph();
	std::pair<std::vector<domain::RouteItem>, double> FindRoute(const std::string& from, const std::string& to);

private:
	const TransportGuide::TransportCatalogue& tc;
	graph::DirectedWeightedGraph<domain::GraphEdge> graph_;
	graph::Router<domain::GraphEdge>* router_ptr{ nullptr };
};

