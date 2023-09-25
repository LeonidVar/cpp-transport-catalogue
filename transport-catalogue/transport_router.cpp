#include "transport_router.h"
#include <iostream>
using namespace TransportGuide;
using namespace domain;

// i_start, i_stop - индексы начальной и конечной остановок на маршруте для добавления в граф
void RouteGraph::GraphAddRouteEdges(graph::DirectedWeightedGraph<GraphEdge>& graph_, const std::vector<std::string_view>& stops_,
	std::string_view bus_, const size_t i_start, const size_t i_stop) {
	// для каждой остановки на маршруте с индексом выхода
	for (size_t i = i_start; i < i_stop; ++i) {
		size_t v1 = tc.stops_index.at(stops_[i]);
		// добавляем ребра до всех следующих остановок с индексом входа
		for (size_t j = i + 1; j < stops_.size(); ++j) {
			size_t v2 = tc.stops_index.at(stops_[j]);
			GraphEdge g_e_;
			g_e_.weight = static_cast<double>(tc.GetRouteLength(bus_, i, j) / tc.bus_velocity_);
			g_e_.bus = bus_;
			g_e_.span_count = j - i;
			graph_.AddEdge({ v1 * 2 + 1, v2 * 2, g_e_ });
			//bus_spans.push_back({ bus_, j - i });
		}
	}
}


void RouteGraph::BuildGraph(){
	graph::DirectedWeightedGraph<GraphEdge> graph_t(tc.stops_data.size() * 2);
	graph_ = std::move(graph_t);
	//std::cout << "GetVertexCount0 " << graph_.GetVertexCount() << std::endl;
	
	// ребра ожидания автобуса на остановке
	for (size_t index = 0; index != tc.stops_data.size(); ++index) {
		graph_.AddEdge({ index * 2, index * 2 + 1,
			{static_cast<double>(tc.bus_wait_time_), 0, ""} });
	}
	//std::cout << "GetVertexCount1 " << graph_.GetVertexCount() << std::endl;
	// ребра между остановками
	for (const auto& [bus_, stops_] : tc.buses_) {
		if (tc.buses_info.at(bus_).is_round_trip) {
			GraphAddRouteEdges(graph_, stops_, bus_, 0, stops_.size() - 1);
		}
		// для не круговых маршрутов то же самое, только как для двух отдельных (туда и обратно)
		else {
			size_t final_i = tc.buses_info.at(bus_).final_stop;
			GraphAddRouteEdges(graph_, stops_, bus_, 0, final_i);
			GraphAddRouteEdges(graph_, stops_, bus_, final_i, stops_.size());
		}
	}

	router_ptr = new graph::Router<GraphEdge>(graph_);
}

void RouteGraph::BuildDeserializedGraph() {
	router_ptr = new graph::Router<GraphEdge>(graph_);
}

std::pair<std::vector<RouteItem>, double> RouteGraph::FindRoute(const std::string& from, const std::string& to) {
	size_t v1 = tc.stops_index.at(from) * 2;
	size_t v2 = tc.stops_index.at(to) * 2;
	auto router_ = router_ptr->BuildRoute(v1, v2);
	if (!router_) return { {}, -1. }; // отрицательная длина => нет маршрута
	double total_time{ 0 };
	std::vector<RouteItem> route_items;
	route_items.reserve(router_->edges.size());
	for (const auto edge_index : router_->edges) {
		auto edge = graph_.GetEdge(edge_index);
		total_time += edge.weight.weight;
		route_items.push_back({ tc.stops_data[edge.from / 2], /*edge.weight.bus*/"777", edge.weight.span_count, edge.weight.weight});
	}

	return { route_items, total_time };
}

graph::DirectedWeightedGraph<domain::GraphEdge>* RouteGraph::GetGraph() {
	return &graph_;
}

graph::Router<domain::GraphEdge>* RouteGraph::GetRouter() {
	return router_ptr;
}
