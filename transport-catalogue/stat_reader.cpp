#include "stat_reader.h"

using namespace std::string_literals;
namespace TransportGuide {
namespace stat {

void ReceiveRequest(const TransportCatalogue& tc, const std::string& s, std::ostream& os) {
	if (s.substr(0, 4) == "Stop"s) {
		PrintStop(tc, s.substr(5), os);
	}
	else if (s.substr(0, 3) == "Bus"s) {
		PrintRoute(tc, s.substr(4), os);
	}
	else {
		os << "Error input!" << std::endl;
	}
}

void PrintRoute(const TransportCatalogue& tc, const std::string& bus, std::ostream& os) {
	if (tc.IsBus(bus)) {
		auto route = tc.GetRouteInfo(bus);
		os << "Bus "s << bus << ": "
			<< route.stops_count << " stops on route, "s
			<< route.unique_stops << " unique stops, "s
			<< route.length << " route length, "s
			<< std::setprecision(6) << route.curvature << " curvature"s << std::endl;
	}
	else {
		os << "Bus "s << bus << ": not found"s << std::endl;
	}
}

void PrintStop(const TransportCatalogue& tc, const std::string& stop, std::ostream& os) {
	std::cout << "Stop "s << stop;

	if (tc.IsStop(stop)) {
		std::set<std::string_view> buses = tc.GetStopInfo(stop);
		if (buses.empty()) {
			os << ": no buses"s << std::endl;
		}
		else {
			std::cout << ": buses"s;
			for (auto& bus : buses) {
				std::cout << " " << bus;
			}
			os << std::endl;
		}
	}
	else {
		os << ": not found"s << std::endl;
	}
}
}
}