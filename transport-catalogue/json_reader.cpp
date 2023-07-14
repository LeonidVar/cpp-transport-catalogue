#include "json_reader.h"

namespace JSON{
using namespace std::string_literals;
using namespace domain;

std::string Print(const json::Node& node) {
    std::ostringstream out;
    Print(json::Document{ node }, out);
    return out.str();
}

svg::Point GetPoint(const json::Array& data) {
    return{ data[0].AsDouble(), data[1].AsDouble() };
}

svg::Color GetColor(const json::Node& color) {
    if (color.IsString()) {
        return { color.AsString() };
    }
    else {
        auto& data = color.AsArray();
        if (data.size() == 3) {
            return { svg::Rgb{ data[0].AsInt(), data[1].AsInt(), data[2].AsInt() } };
        } else {
            return { svg::Rgba{ data[0].AsInt(), data[1].AsInt(), data[2].AsInt(), data[3].AsDouble() } };
        }
    }
}

void LoadFromJson(TransportGuide::TransportCatalogue& tc, std::istream& input, std::ostream& out) {
    json::Document doc = json::Load(input);
    const auto& request = doc.GetRoot().AsMap();
    // Обработка запросов на формирование БД
    BaseRequests(tc, request.at("base_requests"s).AsArray());
    tc.CompleteInput();

    // Обработка запросов на выдачу
    StatRequests(tc, out, request.at("stat_requests"s).AsArray(),
        // Получение параметров SVG и рисование карты в string
        RenderSettings(tc, request.at("render_settings"s).AsMap()));
}

void BaseRequests(TransportGuide::TransportCatalogue& tc, json::Array base_requests) {
    for (const auto& input : base_requests) {
        const auto& cur_map = input.AsMap();
        // Добавление остановки
        if (cur_map.at("type"s).AsString() == "Stop"s) {
            Stop stop;
            // Хранение расстояний между остановками
            std::vector<std::pair<int, std::string>> stops_dist;
            stop.name = cur_map.at("name"s).AsString();
            stop.xy.lat = cur_map.at("latitude"s).AsDouble();
            stop.xy.lng = cur_map.at("longitude"s).AsDouble();
            if (!cur_map.at("road_distances"s).AsMap().empty()) {
                for (const auto& [key, value] : cur_map.at("road_distances"s).AsMap()) {
                    stops_dist.push_back({ value.AsInt(), key });
                }
            }
            tc.AddStopX(stop, stops_dist);
        }
        // Добавление маршрута
        else if (input.AsMap().at("type"s).AsString() == "Bus"s) {
            BusRoute route;
            route.name = cur_map.at("name"s).AsString() ;
            for (const auto& now : cur_map.at("stops"s).AsArray()) {
                route.stops.push_back(now.AsString());
            }
            route.is_round_trip = cur_map.at("is_roundtrip"s).AsBool();
            tc.AddBusTemp(route);
        }
        else {
            throw std::invalid_argument("Error input"s);
        }
    }
}

std::string RenderSettings(TransportGuide::TransportCatalogue& tc, json::Dict render_settings) {
    renderer::Settings rs;
    rs.width = render_settings.at("width"s).AsDouble();
    rs.height = render_settings.at("height"s).AsDouble();
    rs.padding = render_settings.at("padding"s).AsDouble();
    rs.line_width = render_settings.at("line_width"s).AsDouble();
    rs.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    rs.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    rs.bus_label_offset = GetPoint(render_settings.at("bus_label_offset"s).AsArray());
    rs.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
    rs.stop_label_offset = GetPoint(render_settings.at("stop_label_offset"s).AsArray());
    rs.underlayer_color = GetColor(render_settings.at("underlayer_color"s));
    rs.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    const auto& colors = render_settings.at("color_palette"s).AsArray();
    for (const auto& color : colors) {
        rs.color_palette.push_back(GetColor(color));
    }

    std::ostringstream out;
    renderer::RenderRoutes(rs, tc, out);
    return out.str();
}

void StatRequests(TransportGuide::TransportCatalogue& tc, std::ostream& out,
    json::Array stat_requests, std::string svg) {
    json::Array out_print;

    for (const auto& input : stat_requests) {
        json::Dict result;
        const auto& cur_map = input.AsMap();
        if (cur_map.at("type"s).AsString() == "Stop"s) {
            if (tc.IsStop(cur_map.at("name"s).AsString())) {
                std::set<std::string_view> buses = tc.GeStopInfo(cur_map.at("name"s).AsString());
                json::Array stop_arr;
                for (auto& bus : buses) {
                    stop_arr.push_back(std::string(bus));
                }
                result.insert({ {"buses"s, stop_arr}, {"request_id"s, cur_map.at("id"s)} });                
            }
            else {
                result.insert({ {"error_message"s, "not found"s}, {"request_id"s, cur_map.at("id"s)} });
            }
        }
        else if (input.AsMap().at("type"s).AsString() == "Bus"s) {
            if (tc.IsBus(input.AsMap().at("name"s).AsString())) {
                auto route = tc.GetRouteInfo(input.AsMap().at("name"s).AsString());
                result["curvature"s] = route.curvature;
                result["request_id"s] = input.AsMap().at("id"s);
                result["route_length"s] = route.length;
                result["stop_count"s] = route.stops_count;
                result["unique_stop_count"s] = route.unique_stops;
            }
            else {
                result["request_id"s] = input.AsMap().at("id"s);
                result["error_message"s] = "not found"s;
            }
        }
        else if (input.AsMap().at("type"s).AsString() == "Map"s) {
            result["map"s] = svg;
            result["request_id"s] = input.AsMap().at("id"s);
        }
        out_print.push_back(result);
    }
    out << Print(out_print);
}
}