#include "map_renderer.h"

namespace renderer {
using namespace svg;

void MakeText(std::vector<Text>& texts, const Settings& s, const Point pos, 
    const std::string_view data, size_t color, bool is_stop) {
    Text text;
    text.SetPosition(pos)
        .SetOffset(is_stop ? s.stop_label_offset : s.bus_label_offset)
        .SetFontSize(is_stop ? s.stop_label_font_size : s.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor(is_stop ? "black" : s.color_palette[color])
        .SetData(std::string(data));   

    if (!is_stop) text.SetFontWeight("bold");

    Text back_text{ text };
    back_text.SetFillColor(s.underlayer_color)
        .SetStrokeColor(s.underlayer_color)
        .SetStrokeWidth(s.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    texts.push_back(std::move(back_text));
    texts.push_back(std::move(text));
}

void RenderRoutes(Settings& s, TransportGuide::TransportCatalogue& tc, std::ostream& out) {
    svg::Document doc;
    std::vector<geo::Coordinates> coordinates_;;
    auto all_stops = tc.GetStops();
    std::set<std::string_view> this_stops;

    // Формирование списков остановок и их координат для существующих маршрутов
    coordinates_.reserve(all_stops.size());
    const auto& buses_ = tc.GetBuses();
    for (const auto& [bus, stops] : buses_) {
        for (const auto& stop : stops) {
            coordinates_.push_back(all_stops.at(stop));
            this_stops.insert(stop);
        }
    }

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
    coordinates_.begin(), coordinates_.end(), s.width, s.height, s.padding
    };

    size_t i = 0; // счетчик палитры цветов
    std::vector<Text> texts; // названия маршрутов
    std::vector<Text> stop_texts; // названия остановок
    std::vector <Circle> circles;

    for (const auto& stop : this_stops) {
        Circle circle;
        auto point = proj(all_stops.at(stop));
        circle.SetCenter(point).SetRadius(s.stop_radius).SetFillColor("white");
        circles.push_back(std::move(circle));
        MakeText(stop_texts, s, point, stop, i, true);
    }

    for (const auto& [bus, stops] : buses_) {
        Polyline line;
        MakeText(texts, s, proj(all_stops.at(stops.front())), bus, i, false); 
        // Для некольцевых машрутов вторая конечная надпись
        size_t final_stop = tc.GetRouteInfo(std::string(bus)).final_stop;
        if (!tc.GetRouteInfo(std::string(bus)).is_round_trip &&
            stops[final_stop] != stops.front()) {            
            MakeText(texts, s, proj(all_stops.at(stops[final_stop])), bus, i, false);
        }

        for (const auto& stop : stops) {
            auto point = proj(all_stops.at(stop));
            line.AddPoint(point);
        }

        line.SetStrokeColor(s.color_palette[i]);
        line.SetStrokeWidth(s.line_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND)
            .SetFillColor("none");

        doc.Add(std::move(line));
        // Цвета переиспользуются по циклу.
        i < s.color_palette.size() - 1 ? ++i : (i = 0);
    }

    for (auto& text : texts) {
        doc.Add(std::move(text));
    }
    for (auto& circle : circles) {
        doc.Add(std::move(circle));
    }
    for (auto& text : stop_texts) {
        doc.Add(std::move(text));
    }
    doc.Render(out);

}
}