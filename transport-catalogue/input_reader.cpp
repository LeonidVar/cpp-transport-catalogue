#include "input_reader.h"
#include "transport_catalogue.h"

using namespace std::string_literals;
namespace TransportGuide {

void input::Request(TransportCatalogue& tc, std::string s) {
	if (s.substr(0, 4) == "Stop"s) {
		s.erase(0, 5);
		input::StopX(tc, std::move(s));
	}
	else if (s.substr(0, 3) == "Bus"s) {
		s.erase(0, 4);
		input::BusX(tc, std::move(s));
	}
	else {
		std::cout << "Error input!" << std::endl;
	}
}

void input::StopX(TransportCatalogue& tc, std::string s) {
	std::string name = s.substr(0, s.find(':'));
	s.erase(0, s.find(':') + 1);
	double lat = std::stod(s.substr(0, s.find(',')));
	s.erase(0, s.find(',') + 1);
	double lng = std::stod(s.substr(0, s.find(',')));
	if (s.find(',') != std::string::npos)
		s.erase(0, s.find(',') + 2);
	else s.clear();

	//Вектор всех заданных расстояний от текущей остановки до соседних
	std::vector<std::pair<int, std::string>> stops_dist;
	while (!s.empty()) {
		stops_dist.push_back(
			{ stoi(s.substr(0, s.find('m'))),
			s.substr(s.find('m') + 5, s.find(',') - s.find('m') - 5) });
		if (s.find(',') != std::string::npos)
			s.erase(0, s.find(',') + 2);
		else s.clear();
	}

	tc.AddStopX(name, { lat, lng }, stops_dist);
}

void input::BusX(TransportCatalogue& tc, std::string s) {
	std::string name = s.substr(0, s.find(':'));
	s.erase(0, s.find(':') + 2);
	
	std::deque<std::string> stops;
	ReadStopsFromLine(s, '-', stops);
	if (!stops.empty()) {		
		stops.insert(stops.end(), stops.rbegin() + 1, stops.rend());
	}
	else {
		ReadStopsFromLine(s, '>', stops);
	}

	tc.AddBusTemp(name, stops);
}
}