#include "json_reader.h"

namespace JSON{
using namespace std::string_literals;
using namespace domain;

std::string JsonReader::Print(const json::Node& node) {
    std::ostringstream out;
    json::Print(json::Document{ node }, out);
    return out.str();
}

svg::Point JsonReader::GetPoint(const json::Array& data) {
    return{ data[0].AsDouble(), data[1].AsDouble() };
}

svg::Color JsonReader::GetColor(const json::Node& color) {
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

void JsonReader::LoadFromJson() {
    const auto& request = doc.GetRoot().AsDict();
    // Обработка запросов на формирование БД
    BaseRequests(request.at("base_requests"s).AsArray());
    tc.CompleteInput();
    // Получение параметров SVG и рисование карты в string
    RenderSettings(request.at("render_settings"s).AsDict());
    // Обработка запросов на выдачу
    StatRequests(request.at("stat_requests"s).AsArray());       
}

void JsonReader::BaseRequests(const json::Array& base_requests) {
    for (const auto& input : base_requests) {
        const auto& cur_map = input.AsDict();
        // Добавление остановки
        if (cur_map.at("type"s).AsString() == "Stop"s) {
            Stop stop;
            // Хранение расстояний между остановками
            std::vector<std::pair<int, std::string>> stops_dist;
            stop.name = cur_map.at("name"s).AsString();
            stop.xy.lat = cur_map.at("latitude"s).AsDouble();
            stop.xy.lng = cur_map.at("longitude"s).AsDouble();
            if (!cur_map.at("road_distances"s).AsDict().empty()) {
                for (const auto& [key, value] : cur_map.at("road_distances"s).AsDict()) {
                    stops_dist.push_back({ value.AsInt(), key });
                }
            }
            tc.AddStopX(stop, stops_dist);
        }
        // Добавление маршрута
        else if (input.AsDict().at("type"s).AsString() == "Bus"s) {
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

void JsonReader::RenderSettings(const json::Dict& render_settings) {
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
    renderer::MapRenderer mr(rs, tc, out);
    mr.RenderMap();
    svg = std::move(out.str());
}

void JsonReader::StatRequests(const json::Array& stat_requests) {
    json::Array out_print;

    for (const auto& input : stat_requests) {
        json::Builder result;
        result.StartDict();

        const auto& cur_map = input.AsDict();
        if (cur_map.at("type"s).AsString() == "Stop"s) {
            if (tc.IsStop(cur_map.at("name"s).AsString())) {
                std::set<std::string_view> buses = tc.GeStopInfo(cur_map.at("name"s).AsString());
                json::Builder stop_arr_b;
                stop_arr_b.StartArray();
                for (auto& bus : buses) {
                    stop_arr_b.Value(std::string(bus));
                }

                result
                    .Key("buses"s).Value(stop_arr_b.EndArray().Build().AsArray())
                    .Key("request_id"s).Value(cur_map.at("id"s).AsInt());                
            }
            else {
                result
                    .Key("error_message"s).Value("not found"s)
                    .Key("request_id"s).Value(cur_map.at("id"s).AsInt());
            }
        }
        else if (input.AsDict().at("type"s).AsString() == "Bus"s) {
            if (tc.IsBus(input.AsDict().at("name"s).AsString())) {
                auto route = tc.GetRouteInfo(input.AsDict().at("name"s).AsString());
                result
                    .Key("curvature"s).Value(route.curvature)
                    .Key("request_id"s).Value(input.AsDict().at("id"s).AsInt())
                    .Key("route_length"s).Value(route.length)
                    .Key("stop_count"s).Value(route.stops_count)
                    .Key("unique_stop_count"s).Value(route.unique_stops);
            }
            else {
                result
                    .Key("request_id"s).Value(input.AsDict().at("id"s).AsInt())
                    .Key("error_message"s).Value("not found"s);
            }
        }
        else if (input.AsDict().at("type"s).AsString() == "Map"s) {
            result
                .Key("map"s).Value(svg)
                .Key("request_id"s).Value(input.AsDict().at("id"s).AsInt());
        }

        out_print.push_back(result.EndDict().Build().AsDict());
    }
    out << Print(out_print);
}
}