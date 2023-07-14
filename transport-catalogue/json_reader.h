#pragma once
#include <sstream>
#include <string_view>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace JSON {
void LoadFromJson(TransportGuide::TransportCatalogue& tc, std::istream& input, std::ostream& out);
void BaseRequests(TransportGuide::TransportCatalogue& tc, json::Array base_requests);
void StatRequests(TransportGuide::TransportCatalogue& tc, std::ostream& out, json::Array stat_requests, std::string svg);
std::string RenderSettings(TransportGuide::TransportCatalogue& tc, json::Dict render_settings);
std::string Print(const json::Node& node);

}
