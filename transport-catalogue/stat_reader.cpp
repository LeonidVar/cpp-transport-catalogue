#include "stat_reader.h"

using namespace std::string_literals;
namespace TransportGuide {
namespace stat {

void ReceiveRequest(const TransportCatalogue& tc, std::string s) {
	if (s.substr(0, 4) == "Stop"s) {
		PrintStop(tc, s.substr(5));
	}
	else if (s.substr(0, 3) == "Bus"s) {
		PrintRoute(tc, s.substr(4));
	}
	else {
		std::cout << "Error input!" << std::endl;
	}
}

void PrintRoute(const TransportCatalogue& tc, std::string bus) {
	if (tc.IsBus(bus)) {
		auto route = tc.GetRouteInfo(bus);
		std::cout << "Bus "s << bus << ": "
			<< route.stops_count << " stops on route, "s
			<< route.unique_stops << " unique stops, "s
			<< route.length << " route length, "s
			<< std::setprecision(6) << route.curvature << " curvature"s << std::endl;
	}
	else {
		std::cout << "Bus "s << bus << ": not found"s << std::endl;
	}
}

void PrintStop(const TransportCatalogue& tc, std::string stop) {
	std::cout << "Stop "s << stop;

	if (tc.IsStop(stop)) {
		std::set<std::string_view> buses = tc.GeStopInfo(stop);
		if (buses.empty()) {
			std::cout << ": no buses"s << std::endl;
		}
		else {
			std::cout << ": buses"s;
			for (auto bus : buses) {
				std::cout << " " << bus;
			}
			std::cout << std::endl;
		}
	}
	else {
		std::cout << ": not found"s << std::endl;
	}
}
}
}