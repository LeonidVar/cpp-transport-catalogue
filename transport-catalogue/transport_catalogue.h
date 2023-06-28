#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <algorithm>
#include <set>
#include "geo.h"

namespace TransportGuide {
struct RouteInfo {
	int stops_count;
	int unique_stops;
	int length;
	double curvature;
};

class TransportCatalogue {
public:
	//Обработка запроса на добавление остановки
	void AddStopX(std::string name, geo::Coordinates coordinates,
		std::vector<std::pair<int, std::string>> stops_dist);
	//Сформировать базу расстояний между остановками
	//и базу маршрутов, после заполнения всех остановок
	void CompleteInput();
	//Временное хранение маршрутов, до получения всех остановок
	void AddBusTemp(std::string name, std::deque<std::string>);
	//Обработка запроса на добавление маршрута
	void AddBusX(std::string name, std::deque<std::string>);
	//Сформировать информацию о маршруте
	RouteInfo GetRouteInfo(std::string name) const;
	//Сформировать информацию об остановке
	std::set<std::string_view> GeStopInfo(std::string name) const;
	//Проверка наличия автобуса в базе
	bool IsBus(std::string name) const;
	//Проверка наличия остановки в базе
	bool IsStop(std::string name) const;

private:
	//Хешер для stops_distance
	struct PairStopsHasher {
		size_t operator()(const std::pair<std::string_view, std::string_view> stop_par) const {
			return hasher(stop_par.first) + 37 * hasher(stop_par.second);
		}
		std::hash<std::string_view> hasher;
	};

	//Список всех остановок, отсюда берутся указатели
	std::deque<std::string> stops_data;
	//Остановки с географическими координатами
	std::unordered_map<std::string_view, geo::Coordinates> stops_;
	//Остановки с автобусами, которые там проходят
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_buses;
	//Пары остановок с фактическим расстоянием между ними
	std::unordered_map<std::pair<std::string_view, std::string_view>, int, PairStopsHasher> stops_distance;
	//Временное хранение остановки и рассояний до соседей
	std::unordered_map<std::string_view, std::vector<std::pair<int, std::string>>> stop_tmp_dst;
	
	//Временное хранение маршрутов
	std::vector<std::pair<std::string, std::deque<std::string>>> bus_tmp;
	//Список всех автобусов, отсюда берутся указатели
	std::deque<std::string> buses_data;
	//Автобус с остановками на маршруте
	std::unordered_map<std::string_view, std::vector<std::string_view>> buses_;
};
}