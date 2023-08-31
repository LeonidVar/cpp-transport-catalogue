#include "transport_catalogue.h"
#include <iostream>
#include <iterator>

using namespace TransportGuide;
using namespace domain;

void TransportCatalogue::AddStopX(const Stop& stop, std::vector<std::pair<int, std::string>>& stops_dist) {
	stops_data.push_back(stop.name);
	stops_.insert({ stops_data.back(), stop.xy });
	stop_tmp_dst[stops_data.back()] = stops_dist;
}

int TransportCatalogue::GetRouteLength(std::string_view bus_, const size_t from, const size_t to) {
	auto stops_on_route = buses_.at(bus_);
	//int stops_count = stops_on_route.size();
	int length{ 0 };
	for (size_t i = from; i != to; ) {
		//auto check_it = std::find(stops_on_route.begin(), stops_on_route.end(), to);
		auto stop1 = stops_on_route[i];
		++i;
		//if (iter == stops_on_route.end()) break;
		auto stop2 = stops_on_route[i];
		//Поиск прямой или обратной пары остановок
		if (stops_distance.count({ stop1, stop2 })) {
			length += stops_distance.at({ stop1, stop2 });
		}
		else {
			length += stops_distance.at({ stop2, stop1 });
		};
	}
	return length;
	//При наличии для конечных остановок перепробега - добавить расстояние
	//if (stops_distance.count({ *stops_on_route.begin(), *stops_on_route.begin() })) {
	//	real_length += stops_distance.at({ *stops_on_route.begin(), *stops_on_route.begin() });
	//}
	//if (stops_distance.count({ *stops_on_route.rbegin(), *stops_on_route.rbegin() })) {
	//	real_length += stops_distance.at({ *stops_on_route.rbegin(), *stops_on_route.rbegin() });
	//}
}

// i_start, i_stop - индексы начальной и конечной остановок на маршруте для добавления в граф
void TransportCatalogue::GraphAddRouteEdges(graph::DirectedWeightedGraph<GraphEdge>& graph_, const std::vector<std::string_view>& stops_,
	std::string_view bus_, const size_t i_start, const size_t i_stop) {
	// для каждой остановки на маршруте с индексом выхода
	for (size_t i = i_start; i < i_stop; ++i) {
		size_t v1 = stops_index.at(stops_[i]);
		// добавляем ребра до всех следующих остановок с индексом входа
		for (size_t j = i + 1; j < stops_.size(); ++j) {
			size_t v2 = stops_index.at(stops_[j]);
			GraphEdge g_e_;
			g_e_.weight = static_cast<double>(GetRouteLength(bus_, i, j) / bus_velocity_);
			g_e_.bus = bus_;
			g_e_.span_count = j - i;
			graph_.AddEdge({ v1 * 2 + 1, v2 * 2, g_e_ });
			//bus_spans.push_back({ bus_, j - i });
		}
	}
}

void TransportCatalogue::CompleteInput() {
	for (const auto& [first_stop, distances] : stop_tmp_dst) {
		for (const auto& [ dist, second_stop ]:distances) {
			std::string_view sec_stop = *std::find(stops_data.begin(), stops_data.end(), second_stop);
			stops_distance[{first_stop, sec_stop}] = dist;
		}
	}

	for (auto& route : bus_tmp) {
		AddBusX(route);
	}

	for (size_t i = 0; i != stops_data.size(); ++i) {
		stops_index[stops_data[i]] = i;
	}

	//std::vector<std::pair<std::string_view, int>> stop_spans;
	//std::vector<std::pair<std::string_view, int>> bus_spans;
	graph::DirectedWeightedGraph<GraphEdge> graph_t(stops_data.size() * 2);
	graph_ = std::move(graph_t);
	for (size_t index = 0; index != stops_data.size(); ++index) {		
		graph_.AddEdge({ index * 2, index * 2 + 1, 
			{static_cast<double>(bus_wait_time_), 0, ""} });
		//stop_spans.push_back({ std::string_view(stops_data[index]), 0});
	}
	for (const auto& [bus_, stops_] : buses_) {
		if (buses_info[bus_].is_round_trip) {
			GraphAddRouteEdges(graph_, stops_, bus_, 0, stops_.size() - 1);
		}
		// для не круговых маршрутов то же самое, только как для двух отдельных (туда и обратно)
		else {
			size_t final_i = buses_info[bus_].final_stop;
			GraphAddRouteEdges(graph_, stops_, bus_, 0, final_i);
			GraphAddRouteEdges(graph_, stops_, bus_, final_i, stops_.size());
		}
	}
}

std::pair<std::vector<RouteItem>, double> TransportCatalogue::FindRoute(const std::string& from, const std::string& to) {
	graph::Router<GraphEdge> router(graph_);
	size_t v1 = stops_index.at(from) * 2;
	size_t v2 = stops_index.at(to) * 2;
	//graph::Router::RouteInfo ri;
	auto router_ = router.BuildRoute(v1, v2);

	if (!router_) return { {}, -1. };
	
	double total_time{ 0 };
	std::vector<RouteItem> route_items;
	route_items.reserve(router_->edges.size());
	//RouteItem ri;
	//ri.start_stop = from;
	//ri.time = bus_wait_time_;
	//route_items.push_back(ri);

	for (const auto edge_index : router_->edges) {
		auto edge = graph_.GetEdge(edge_index);
		total_time += edge.weight.weight;
		route_items.push_back({ stops_data[edge.from / 2], edge.weight.bus, edge.weight.span_count, edge.weight.weight});

	}
	return { route_items, total_time };
}

void TransportCatalogue::AddBusTemp(BusRoute& route) {
	bus_tmp.push_back(std::move(route));
}

void TransportCatalogue::AddBusX(BusRoute& route) {
	buses_data.push_back(route.name);
	std::vector<std::string_view> stops_sv;
	// Номер конечной остановки, используется для некольцевых маршрутов
	size_t final_stop{ route.stops.size() };

	for (std::string& stop : route.stops) {
		std::string_view stop_sv = *std::find(stops_data.begin(), stops_data.end(), stop);
		stops_buses[stop_sv].insert(buses_data.back());
		stops_sv.push_back(stop_sv);
	}
	buses_.insert({ buses_data.back(), stops_sv });

	// Для некольцевого машрута остановки добавляются в обратном порядке
	if (!route.is_round_trip) {
		buses_.at(route.name).insert(
			buses_.at(route.name).end(), stops_sv.rbegin() + 1, stops_sv.rend());
	}

	//Подсчет и сохарнение статистики по маршруту
	auto stops_on_route = buses_.at(route.name);
	int stops_count = stops_on_route.size();
	double geo_length{ 0 };
	int real_length{ 0 };
	for (auto iter = stops_on_route.begin(); iter != --stops_on_route.end(); ) {
		auto stop1 = *iter;
		geo::Coordinates s1 = stops_.at(stop1);
		++iter;
		auto stop2 = *iter;
		geo::Coordinates s2 = stops_.at(stop2);
		geo_length += geo::ComputeDistance(s1, s2);
		//Поиск прямой или обратной пары остановок
		if (stops_distance.count({ stop1, stop2 })) {
			real_length += stops_distance.at({ stop1, stop2 });
		}
		else {
			real_length += stops_distance.at({ stop2, stop1 });
		};
	}
	//При наличии для конечных остановок перепробега - добавить расстояние
	if (stops_distance.count({ *stops_on_route.begin(), *stops_on_route.begin() })) {
		real_length += stops_distance.at({ *stops_on_route.begin(), *stops_on_route.begin() });
	}
	if (stops_distance.count({ *stops_on_route.rbegin(), *stops_on_route.rbegin() })) {
		real_length += stops_distance.at({ *stops_on_route.rbegin(), *stops_on_route.rbegin() });
	}
	double curvature = real_length / geo_length;

	//Подсчет числа уникальных остановок
	sort(stops_on_route.begin(), stops_on_route.end());
	int unique_stops = unique(stops_on_route.begin(), stops_on_route.end()) - stops_on_route.begin();

	buses_info[buses_data.back()] = { stops_count, unique_stops, real_length, curvature,
									route.is_round_trip, final_stop - 1 };
}

bool TransportCatalogue::IsBus(const std::string& name) const {
	return (std::find(buses_data.begin(), buses_data.end(), name) != buses_data.end());
}

bool TransportCatalogue::IsStop(const std::string& name) const {
	return (std::find(stops_data.begin(), stops_data.end(), name) != stops_data.end());
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string& name) const {
	return buses_info.at(name);
}

std::set<std::string_view> TransportCatalogue::GeStopInfo(const std::string& name) const {
	if (stops_buses.count(name)) {
		return stops_buses.at(name);
	}
	else {
		return {};
	}
}

std::map<std::string_view, std::vector<std::string_view>> TransportCatalogue::GetBuses()  const {
	return buses_;
}
std::unordered_map<std::string_view, geo::Coordinates> TransportCatalogue::GetStops()  const {
	return stops_;
}