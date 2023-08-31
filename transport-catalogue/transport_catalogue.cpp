#include "transport_catalogue.h"
#include <iostream>
#include <iterator>
#include "transport_router.h"

using namespace TransportGuide;
using namespace domain;

void TransportCatalogue::AddStopX(const Stop& stop, std::vector<std::pair<int, std::string>>& stops_dist) {
	stops_data.push_back(stop.name);
	stops_.insert({ stops_data.back(), stop.xy });
	stop_tmp_dst[stops_data.back()] = stops_dist;
}

int TransportCatalogue::GetRouteLength(const std::string_view bus_, const size_t from, const size_t to) const {
	auto& stops_on_route = buses_.at(bus_);
	int length{ 0 };
	for (size_t i = from; i != to; ) {
		auto stop1 = stops_on_route[i];
		++i;
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

void TransportCatalogue::CompleteInput() {
	for (const auto& [first_stop, distances] : stop_tmp_dst) {
		for (const auto& [dist, second_stop] : distances) {
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

	route_graph_ptr = new RouteGraph(*this);
	route_graph_ptr->BuildGraph();	
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

const RouteGraph& TransportCatalogue::GetRouteGraph() const {
	return *route_graph_ptr;
}