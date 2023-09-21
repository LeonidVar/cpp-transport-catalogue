#pragma once
#include <sstream>
#include <string_view>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>
#include "json.h"
#include "json_builder.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <filesystem>
#include "serialization.h"

namespace JSON {

class JsonReader{
public:
	JsonReader(TransportGuide::TransportCatalogue& tc_in, std::istream& input_in, std::ostream& out_in)
		: tc(tc_in), input(input_in), out(out_in) {}

	void MakeBase();
	void ProcessRequests();

private:
	
	void BaseRequests(const json::Array& base_requests);
	void RoutingRequest(const json::Dict& routing_settings);
	void MakeBaseSerializationRequests(const json::Dict& serialization_settings);
	void ProcessSerializationRequests(const json::Dict& serialization_settings);
	void StatRequests(const json::Array& stat_requests);
	void RenderSettings(const json::Dict& render_settings);
	std::string Print(const json::Node& node);
	svg::Point GetPoint(const json::Array& data);
	svg::Color GetColor(const json::Node& color);

	TransportGuide::TransportCatalogue& tc;
	std::istream& input;
	std::ostream& out;
	json::Document doc{ json::Load(input) };
	std::string svg;
	std::filesystem::path in_file;
	std::filesystem::path out_file;
};


}
