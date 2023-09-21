#include "serialization.h"
#include <filesystem>
#include <fstream>


void Serialize(const TransportGuide::TransportCatalogue& tc, const std::filesystem::path& path) {
    auto stops = tc.GetStops();
    auto buses = tc.GetBuses();
    auto distances = tc.GetStopDistances();

    bus_serialize::TransportCatalogue sz_tc;

    for (const auto [stop_name, coordinates] : stops) {
        size_t i = 0;
        bus_serialize::Stop sz_stop;
        sz_stop.set_name(static_cast<std::string>(stop_name));
        sz_stop.set_lat(coordinates.lat);
        sz_stop.set_lng(coordinates.lng);
        sz_stop.set_id(tc.GetStopId(stop_name));
        *sz_tc.mutable_stops(i) = std::move(sz_stop);
        ++i;
    }

    for (const auto& [bus, stops] : buses) {
        auto ri = tc.GetRouteInfo(bus);
        size_t i = 0;
        bus_serialize::Bus sz_bus;
        sz_bus.set_route(static_cast<std::string>(bus));
        sz_bus.set_is_roundtrip(ri.is_round_trip);
        for (size_t j = 0; j <= ri.final_stop; ++j) {
            sz_bus.add_stops_id(tc.GetStopId(stops[i]));
        }
        *sz_tc.mutable_buses(i) = std::move(sz_bus);
        ++i;
    }

    for (const auto& [stops, dist] : distances) {
        size_t i = 0;
        bus_serialize::Distance sz_dist;
        sz_dist.set_distance(dist);
        sz_dist.set_from_id(tc.GetStopId(stops.first));
        sz_dist.set_to_id(tc.GetStopId(stops.second));
        *sz_tc.mutable_stop_distances(i) = std::move(sz_dist);
        ++i;
    }

    std::ofstream out_file(path, std::ios::binary);
    sz_tc.SerializeToOstream(&out_file);

}

void Deserialize(TransportGuide::TransportCatalogue& tc, const std::filesystem::path& path) {
    std::ifstream in_file(path, std::ios::binary);
    bus_serialize::TransportCatalogue sz_tc;
    sz_tc.ParseFromIstream(&in_file);
    //if (!sz_tc.ParseFromIstream(&in_file)) {
    //    return nullopt;
    //}

    for (auto& sz_dist : sz_tc.stop_distances()) {
        tc.AddDsrlzDistance(sz_dist.from_id(), sz_dist.to_id(), sz_dist.distance());
    }

    for (auto& sz_stop : sz_tc.stops()) {
        //domain::Stop{ sz_stop.name(), {sz_stop.lat(), sz_stop.lng()} };
        tc.AddDsrlzStop(domain::Stop{ sz_stop.name(), {sz_stop.lat(), sz_stop.lng()} },
            sz_stop.id());
    }

    for (auto& sz_bus : sz_tc.buses()) {
        std::vector<std::string> stops;
        for (auto& sz_stop_id : sz_bus.stops_id()) {
            stops.push_back(tc.GetStopName(sz_stop_id));
        }
        domain::BusRoute result{ sz_bus.route(), stops, sz_bus.is_roundtrip() };
        tc.AddBusX(result);
    }
}