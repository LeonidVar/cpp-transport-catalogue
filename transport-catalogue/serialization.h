#pragma once
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "domain.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include <filesystem>
#include "graph.h"

class Serializator {
public:
	Serializator(TransportGuide::TransportCatalogue& tc, const std::filesystem::path& path):
		tc_(tc), path_(path) {
        stops = tc_.GetStops();
        buses = tc_.GetBuses();
        distances = tc_.GetStopDistances();
    }

    void SerializeStops();
    void SerializeBuses();
    void SerializeDistances();
    void SerializeRenderSettings(renderer::Settings& rs);
    void SerializeGraph();
    void MakeDataFile();

    void Deserialize();
    renderer::Settings DeserializeRenderSettings();
    void DeserializeRouter();

    svg::Color ProtoGetColor(const bus_serialize::Color& p_color);


private:
	TransportGuide::TransportCatalogue& tc_;
	const std::filesystem::path path_;
    bus_serialize::TransportCatalogue sz_tc;
    bus_serialize::RenderSettings* render_ = sz_tc.mutable_render_settings();
    std::unordered_map <std::string_view, geo::Coordinates> stops;
    std::map<std::string_view, std::vector<std::string_view>> buses;
    std::unordered_map<std::pair<std::string_view, std::string_view>, int, domain::PairStopsHasher> distances;
    
};

