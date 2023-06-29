#include "transport_catalogue.h"
#include <iostream>

using namespace TransportGuide;
void TransportCatalogue::AddStopX(const std::string& name,
	geo::Coordinates coordinates, std::vector<std::pair<int, std::string>> stops_dist) {
	stops_data.push_back(name);
	stops_.insert({ stops_data.back(), coordinates});
	stop_tmp_dst[stops_data.back()] = stops_dist;
}

void TransportCatalogue::CompleteInput() {
	for (const auto& [first_stop, distances] : stop_tmp_dst) {
		for (const auto& [ dist, second_stop ]:distances) {
			std::string_view sec_stop = *std::find(stops_data.begin(), stops_data.end(), second_stop);
			stops_distance[{first_stop, sec_stop}] = dist;
		}
	}

	for (auto [bus, stops] : bus_tmp) {
		AddBusX(bus, stops);
	}


}

void TransportCatalogue::AddBusTemp(const std::string& name, std::deque<std::string> stops) {
	bus_tmp.push_back({ std::move(name), std::move(stops) });
}

void TransportCatalogue::AddBusX(const std::string& name, std::deque<std::string> stops) {
	buses_data.push_back(name);
	std::vector<std::string_view> stops_sv;
	for (std::string& stop : stops) {
		std::string_view stop_sv = *std::find(stops_data.begin(), stops_data.end(), stop);
		stops_buses[stop_sv].insert(buses_data.back());
		stops_sv.push_back(stop_sv);
	}
	buses_.insert({ buses_data.back(), stops_sv });

	//Подсчет и сохарнение статистики по маршруту
	auto stops_on_route = buses_.at(name);
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

	buses_info[buses_data.back()] = { stops_count, unique_stops, real_length, curvature };
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