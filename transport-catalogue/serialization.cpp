#include "serialization.h"
#include "router.h"
#include <filesystem>
#include <fstream>

void Serializator::MakeDataFile() {
    std::ofstream out_file(path_, std::ios::binary);
    sz_tc.SerializeToOstream(&out_file);
}

void Serializator::SerializeStops() {
    for (size_t i = 0; i < stops.size(); ++i) {
        const auto stop_name = tc_.GetStopName(i);
        const auto coordinates = stops.at(stop_name);
        bus_serialize::Stop* sz_stop = sz_tc.add_stops();
        sz_stop->set_name(static_cast<std::string>(stop_name));
        sz_stop->set_lat(coordinates.lat);
        sz_stop->set_lng(coordinates.lng);
        sz_stop->set_id(tc_.GetStopId(stop_name));
    }
}

void Serializator::SerializeBuses() {
    for (const auto& [bus, stops] : buses) {
        auto ri = tc_.GetRouteInfo(bus);
        bus_serialize::Bus* sz_bus = sz_tc.add_buses();
        sz_bus->set_route(static_cast<std::string>(bus));
        sz_bus->set_is_roundtrip(ri.is_round_trip);
        for (size_t j = 0; j <= ri.final_stop; ++j) {
            sz_bus->add_stops_id(tc_.GetStopId(stops[j]));
        }
    }
}

void Serializator::SerializeDistances() {
    for (const auto& [stops, dist] : distances) {
        bus_serialize::Distance* sz_dist = sz_tc.add_stop_distances();
        sz_dist->set_distance(dist);
        sz_dist->set_from_id(tc_.GetStopId(stops.first));
        sz_dist->set_to_id(tc_.GetStopId(stops.second));
    }
}

void Serializator::SerializeRenderSettings(renderer::Settings& rs) {
    render_->set_width(rs.width);
    render_->set_height(rs.height);
    render_->set_padding(rs.padding);
    render_->set_line_width(rs.line_width);
    render_->set_stop_radius(rs.stop_radius);
    render_->set_bus_label_font_size(rs.bus_label_font_size);
    render_->mutable_bus_label_offset()->set_x(rs.bus_label_offset.x);
    render_->mutable_bus_label_offset()->set_y(rs.bus_label_offset.y);
    render_->set_stop_label_font_size(rs.stop_label_font_size);
    render_->mutable_stop_label_offset()->set_x(rs.stop_label_offset.x);
    render_->mutable_stop_label_offset()->set_y(rs.stop_label_offset.y);
 
    if (std::holds_alternative<std::string>(rs.underlayer_color)) {
        render_->mutable_underlayer_color()->set_string_color(std::get<std::string>(rs.underlayer_color));
    }
    else if (std::holds_alternative<svg::Rgb>(rs.underlayer_color)) {
        svg::Rgb c = std::get<svg::Rgb>(rs.underlayer_color);
        auto rgb = render_->mutable_underlayer_color()->mutable_rgb_color();
        rgb->set_r(c.red);
        rgb->set_g(c.green);
        rgb->set_b(c.blue);
    }
    else if (std::holds_alternative<svg::Rgba>(rs.underlayer_color)) {
        svg::Rgba c = std::get<svg::Rgba>(rs.underlayer_color);
        auto rgba = render_->mutable_underlayer_color()->mutable_rgba_color();
        rgba->set_r(c.red);
        rgba->set_g(c.green);
        rgba->set_b(c.blue);
        rgba->set_o(c.opacity);
    }
    
    render_->set_underlayer_width(rs.underlayer_width);
    
    for (const auto& c : rs.color_palette) {
        auto color_ = render_->add_color_palette();
            
        if (std::holds_alternative<std::string>(c)) {
            color_->set_string_color(std::get<std::string>(c));

        }
        else if (std::holds_alternative<svg::Rgb>(c)) {
            svg::Rgb rgb_c = std::get<svg::Rgb>(c);
            color_->mutable_rgb_color()->set_r(rgb_c.red);
            color_->mutable_rgb_color()->set_g(rgb_c.green);
            color_->mutable_rgb_color()->set_b(rgb_c.blue);

        }
        else if (std::holds_alternative<svg::Rgba>(c)) {
            svg::Rgba rgba_c = std::get<svg::Rgba>(c);
            color_->mutable_rgba_color()->set_r(rgba_c.red);
            color_->mutable_rgba_color()->set_g(rgba_c.green);
            color_->mutable_rgba_color()->set_b(rgba_c.blue);
            color_->mutable_rgba_color()->set_o(rgba_c.opacity);
        }
    }
}

void Serializator::Deserialize() {
    std::ifstream in_file(path_, std::ios::binary);
    sz_tc.ParseFromIstream(&in_file);

    for (auto& sz_stop : sz_tc.stops()) {
        tc_.AddDsrlzStop(domain::Stop{ sz_stop.name(), {sz_stop.lat(), sz_stop.lng()} },
            sz_stop.id());

    }

    for (auto& sz_dist : sz_tc.stop_distances()) {
        tc_.AddDsrlzDistance(sz_dist.from_id(), sz_dist.to_id(), sz_dist.distance());

    }

    for (auto& sz_bus : sz_tc.buses()) {
        std::vector<std::string> stops;
        for (auto& sz_stop_id : sz_bus.stops_id()) {
            stops.push_back(tc_.GetStopName(sz_stop_id));
        }
        domain::BusRoute result{ sz_bus.route(), stops, sz_bus.is_roundtrip() };
        tc_.AddBusX(result);
    }
}

renderer::Settings Serializator::DeserializeRenderSettings() {
    auto sz_rs = sz_tc.render_settings();

    renderer::Settings rs;
    rs.width = sz_rs.width();
    rs.height = sz_rs.height();
    rs.padding = sz_rs.padding();
    rs.line_width = sz_rs.line_width();
    rs.stop_radius = sz_rs.stop_radius();
    rs.bus_label_font_size = sz_rs.bus_label_font_size();
    rs.bus_label_offset.x = sz_rs.bus_label_offset().x();
    rs.bus_label_offset.y = sz_rs.bus_label_offset().y();
    rs.stop_label_font_size = sz_rs.stop_label_font_size();
    rs.stop_label_offset.x = sz_rs.stop_label_offset().x();
    rs.stop_label_offset.y = sz_rs.stop_label_offset().y();
    rs.underlayer_color = ProtoGetColor(sz_rs.underlayer_color());
    rs.underlayer_width = sz_rs.underlayer_width();

    rs.color_palette.reserve(sz_rs.color_palette().size());
    for (size_t i = 0; i < sz_rs.color_palette_size(); i++) {
        rs.color_palette.push_back(ProtoGetColor(sz_rs.color_palette(i)));
    }

    return rs;
}

svg::Color Serializator::ProtoGetColor(const bus_serialize::Color & p_color) {
    svg::Color color;
    switch (p_color.color_case()) {
    case bus_serialize::Color::kStringColor:
        color = p_color.string_color();
        break;
    case bus_serialize::Color::kRgbColor:
    {
        auto& proto_rgb = p_color.rgb_color();
        color = svg::Rgb(proto_rgb.r(), proto_rgb.g(), proto_rgb.b());
    }
    break;
    case bus_serialize::Color::kRgbaColor:
    {
        auto& proto_rgba = p_color.rgba_color();
        color = svg::Rgba(proto_rgba.r(), proto_rgba.g(), proto_rgba.b(), proto_rgba.o());
    }
    break;
    default:
        color = svg::NoneColor;
    }
    return color;
}

void Serializator::SerializeGraph() {
    auto graph = tc_.GetRouteGraph()->GetGraph();
    auto router = tc_.GetRouteGraph()->GetRouter();

    router_serialize::TransportRouter* sz_tr = sz_tc.mutable_router();

    auto* sz_tr_settings = sz_tr->mutable_settings();
    sz_tr_settings->set_bus_velocity(tc_.bus_velocity_);
    sz_tr_settings->set_bus_wait_time(tc_.bus_wait_time_);

    auto* sz_tr_graph = sz_tr->mutable_graph();

    size_t graph_size = graph->GetEdgeCount();

    for (size_t i = 0; i < graph_size; ++i) {
        auto edge = graph->GetEdge(i);
        router_serialize::Edge sz_edge;
        sz_edge.set_vertex_id_from(edge.from);
        sz_edge.set_vertex_id_to(edge.to);
        sz_edge.mutable_weight()->set_bus_name(static_cast<std::string>(edge.weight.bus));
        sz_edge.mutable_weight()->set_weight(edge.weight.weight);
        sz_edge.mutable_weight()->set_span_count(edge.weight.span_count);
        *sz_tr_graph->add_edges() = std::move(sz_edge);
    }

    size_t vertex_size = graph->GetVertexCount();

    for (size_t i = 0; i < vertex_size; ++i) {
        auto incidence_list = graph->GetIncidentEdges(i);
        auto sz_list = sz_tr_graph->add_incidence_list();
        for (auto id : incidence_list) {
            sz_list->add_edges_id(id);
        }
    }

    
    //auto* sz_tr_router = sz_tr->mutable_router();

    //    for (const auto& routes_data : *router->GetRoutesInternalData()) {
    //        router_serialize::RoutesInternalData sz_data;
    //        for (const auto& route_data : routes_data) {
    //            router_serialize::OptionalRouteInternalData sz_internal;
    //            if (route_data.has_value()) {
    //                auto& value = route_data.value();
    //                auto sz_value = sz_internal.mutable_data();
    //                sz_value->mutable_route_weight()->set_weight(value.weight.weight);
    //                if (value.prev_edge.has_value()) {
    //                    sz_value->mutable_prev_edge()->set_edge_id(value.prev_edge.value());
    //                }
    //            }
    //            *sz_data.add_routes_internal_data() = std::move(sz_internal);
    //        }
    //    *sz_tr_router->add_routes_data() = std::move(sz_data);
    //    }
 
}


void Serializator::DeserializeRouter() {
    tc_.SetRoutePtr();
    RouteGraph* route_graph_ptr = tc_.GetRouteGraph();
    auto graph = route_graph_ptr->GetGraph();

    graph::DirectedWeightedGraph<domain::GraphEdge> graph_t(tc_.GetStops().size() * 2);
    *graph = std::move(graph_t);

    router_serialize::RouteSettings sz_rs = sz_tc.router().settings();
    tc_.bus_velocity_ = sz_rs.bus_velocity();   
    tc_.bus_wait_time_ = sz_rs.bus_wait_time(); 

    auto sz_graph = sz_tc.router().graph();
    auto edge_count = sz_graph.edges_size();

    for (size_t i = 0; i < edge_count; ++i) {
        graph::Edge<domain::GraphEdge> edge;
        auto& sz_edge = sz_graph.edges(i);
        edge.from = sz_edge.vertex_id_from();
        edge.to = sz_edge.vertex_id_to();
        edge.weight = domain::GraphEdge{ sz_edge.weight().weight(),
            static_cast<int>(sz_edge.weight().span_count()),
            sz_edge.weight().bus_name() };
        graph->GetEdges().push_back(edge);
    }

    auto incidence_lists_count = sz_graph.incidence_list_size();
    for (size_t i = 0; i < incidence_lists_count; ++i) {
        std::vector<graph::EdgeId> list;
        auto& sz_incidence_list = sz_graph.incidence_list(i);
        auto list_edges_count = sz_incidence_list.edges_id_size();
        auto graph_incidence_lists = graph->GetIncidenceLists();

        for (size_t j = 0; j < list_edges_count; ++j) {
            list.push_back(sz_incidence_list.edges_id(j));
        }
        graph_incidence_lists.push_back(list);
    }

    //route_graph_ptr->BuildDeserializedGraph();

    //auto sz_router = sz_tc.router().router();
    //auto routes_internal_data_count = sz_router.routes_data_size();

    //auto router = route_graph_ptr->GetRouter();
    //auto ri_data = router->GetRoutesInternalData();

    //for (size_t i = 0; i < routes_internal_data_count; ++i) {
    //    auto& sz_internal_data = sz_router.routes_data(i);
    //    auto internal_data_count = sz_internal_data.routes_internal_data_size();
    //    for (size_t j = 0; j < internal_data_count; ++j) {
    //        auto& sz_optional_data = sz_internal_data.routes_internal_data(j);
    //        if (sz_optional_data.has_data()) {               
    //            graph::Router<domain::GraphEdge>::RouteInternalData data;
    //            auto& sz_data = sz_optional_data.data();
    //            data.weight.weight = sz_data.route_weight().weight();
    //            if (sz_data.has_prev_edge()) {
    //                data.prev_edge = sz_data.prev_edge().edge_id();
    //            }
    //            else {
    //                data.prev_edge = std::nullopt;
    //            }
    //            (*ri_data)[i][j] = std::move(data);
    //        }
    //        else {               
    //            (*ri_data)[i][j] = std::nullopt;
    //        }
    //    }
    //}


    //std::cout << "BuildGraph" << std::endl;
    route_graph_ptr->BuildGraph();
}
