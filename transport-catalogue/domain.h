#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <algorithm>
#include <set>
#include "geo.h"

namespace domain {

	struct Stop {
		std::string name;
		geo::Coordinates xy;
	};

	struct BusRoute {
		std::string name;
		std::vector<std::string> stops;
		bool is_round_trip;
	};

	struct RouteInfo {
		int stops_count;
		int unique_stops;
		int length;
		double curvature;
		bool is_round_trip;
		size_t final_stop; // позиция 2ой конечной оствановки для некольцевых маршрутов
	};

	struct GraphEdge {
		double weight;
		int span_count;
		std::string_view bus;

		bool operator< (const GraphEdge rhs) const {
			return weight < rhs.weight;
		}
		bool operator> (const GraphEdge rhs) const {
			return weight > rhs.weight;
		}
		GraphEdge operator+ (const GraphEdge rhs) const {
			return { weight + rhs.weight, span_count, bus };
		}
	};

	struct RouteItem {
		std::string_view start_stop;
		std::string_view bus;
		int span_count{ 0 };
		double time;
	};

	//Хешер для stops_distance
	struct PairStopsHasher {
		size_t operator()(const std::pair<std::string_view, std::string_view> stop_par) const {
			return hasher(stop_par.first) + 37 * hasher(stop_par.second);
		}
		std::hash<std::string_view> hasher;
	};

}

