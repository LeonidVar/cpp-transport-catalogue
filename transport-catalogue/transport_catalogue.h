#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <algorithm>
#include <set>
#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "router.h"
//#include "serialization.h"
//#include "transport_router.h"

class RouteGraph;

namespace TransportGuide {

class TransportCatalogue {
public:
	friend class ::RouteGraph;

	//Обработка запроса на добавление остановки
	void AddStopX(const domain::Stop& stop, std::vector<std::pair<int, std::string>>& stops_dist);
	//Сформировать базу расстояний между остановками
	//и базу маршрутов, после заполнения всех остановок.
	void CompleteInput();
	//Временное хранение маршрутов, до получения всех остановок
	void AddBusTemp(domain::BusRoute&);
	//Обработка запроса на добавление маршрута
	//Подсчет статистики по каждому маршруту.
	void AddBusX(domain::BusRoute& route);
	//Выдать информацию о маршруте
	domain::RouteInfo GetRouteInfo(const std::string_view) const;
	//Сформировать информацию об остановке
	std::set<std::string_view> GetStopInfo(const std::string&) const;
	//Проверка наличия автобуса в базе
	bool IsBus(const std::string&) const;
	//Проверка наличия остановки в базе
	bool IsStop(const std::string&) const;
	//Кольцевой маршрут или нет
	bool IsRoundRoute(const std::string_view bus_) const;
	

	// подсчет расстояния между остановками на маршруте
	int GetRouteLength(const std::string_view bus_, const size_t i_start, const size_t i_stop) const;

	size_t GetStopId(const std::string_view stop) const;
	std::string GetStopName(size_t id) const;

	size_t GetBusId(const std::string_view bus) const;
	std::string GetBusName(size_t id) const;

	RouteGraph* GetRouteGraph();

	// Выдача маршрутов с остановками
	std::map<std::string_view, std::vector<std::string_view>> GetBuses() const;
	// Выдача остановок с координатами
	std::unordered_map<std::string_view, geo::Coordinates> GetStops() const;
	// Выдача расстояний между остановками
	std::unordered_map<std::pair<std::string_view, std::string_view>, int, domain::PairStopsHasher> GetStopDistances() const;

	void AddDsrlzStop(const domain::Stop& stop, size_t id);
	//void AddDsrlzBus(std::string name, std::vector<int> stops, bool is_roundtrip);
	void AddDsrlzDistance(int from_id, int to_id, int dist);

	//время ожидания автобуса на остановке, в минутах
	int bus_wait_time_{ 0 };
	//скорость автобуса, в км/ч
	double bus_velocity_;


private:
	//Список всех остановок, отсюда берутся указатели
	std::deque<std::string> stops_data;
	//Номера остановок
	std::unordered_map<std::string_view, size_t> stops_index;
	std::unordered_map<std::string_view, size_t> buses_index;
	//Координаты всех остановок
	std::vector<geo::Coordinates> coordinates_;
	//Остановки с географическими координатами
	std::unordered_map<std::string_view, geo::Coordinates> stops_;
	//Остановки с автобусами, которые там проходят
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_buses;
	//Пары остановок с фактическим расстоянием между ними
	std::unordered_map<std::pair<std::string_view, std::string_view>, int, domain::PairStopsHasher> stops_distance;
	//Временное хранение остановки и рассояний до соседей
	std::unordered_map<std::string_view, std::vector<std::pair<int, std::string>>> stop_tmp_dst;

	//Временное хранение маршрутов
	std::vector<domain::BusRoute> bus_tmp;
	//Список всех автобусов, отсюда берутся указатели
	std::deque<std::string> buses_data;
	//Автобус с остановками на маршруте
	std::map<std::string_view, std::vector<std::string_view>> buses_;
	//Автобус с информацией о маршруте
	//std::unordered_map<std::string_view, RouteInfo> buses_info;
	std::unordered_map<std::string_view, domain::RouteInfo> buses_info;
	
	RouteGraph* route_graph_ptr{ nullptr };

public:
	void SetRoutePtr();
};
}