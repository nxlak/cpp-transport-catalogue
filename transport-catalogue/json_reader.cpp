#include "json_reader.h"

namespace json_reader {

void JsonReader::ProcessRenderSettings(const json::Node& node, RenderSettings& settings) {
    settings.width = node.AsMap().at("width").AsDouble();
    settings.height = node.AsMap().at("height").AsDouble();
    settings.padding = node.AsMap().at("padding").AsDouble();
    settings.line_width = node.AsMap().at("line_width").AsDouble();
    settings.stop_radius = node.AsMap().at("stop_radius").AsDouble();
    settings.bus_label_font_size = node.AsMap().at("bus_label_font_size").AsInt();
    settings.bus_label_offset = {node.AsMap().at("bus_label_offset").AsArray().at(0).AsDouble(), 
                                  node.AsMap().at("bus_label_offset").AsArray().at(1).AsDouble()};
    settings.stop_label_font_size = node.AsMap().at("stop_label_font_size").AsInt();
    settings.stop_label_offset = {node.AsMap().at("stop_label_offset").AsArray().at(0).AsDouble(), 
                                  node.AsMap().at("stop_label_offset").AsArray().at(1).AsDouble()};
    settings.underlayer_color = node.AsMap().at("underlayer_color");
    settings.underlayer_width = node.AsMap().at("underlayer_width").AsDouble();
    settings.color_palette = node.AsMap().at("color_palette").AsArray();
} 
    
void JsonReader::ProcessStateRequest(const json::Node& node, TransportCatalogue& catalogue, std::vector<json::Node>& responses/*, const RenderSettings& settings*/, std::string map_json) {
    const auto& type = node.AsMap().at("type").AsString();
    const auto& id = node.AsMap().at("id").AsInt();

    json::Node response(json::Dict{});
    response.AsMap()["request_id"] = id;
    
    if (type == "Bus") {
        const auto& bus_name = node.AsMap().at("name").AsString();

        auto bus_info = catalogue.GetBusInfo(bus_name);
        if (bus_info) {
            response.AsMap()["curvature"] = bus_info->curvature;
            response.AsMap()["route_length"] = bus_info->distance;
            response.AsMap()["stop_count"] = bus_info->stops_count;
            response.AsMap()["unique_stop_count"] = bus_info->unique_stops_count;
        } else {
            std::string s = "not found";
            response.AsMap()["error_message"] = s;
        }
    } else if (type == "Stop") {
        const auto& stop_name = node.AsMap().at("name").AsString();
        const Stop* stop_ptr = catalogue.FindStop(stop_name);
        if (!stop_ptr) {
            std::string s = "not found";
            response.AsMap()["error_message"] = s;
        }
        else {
            auto buses = catalogue.GetBusesByStop(stop_name);
            json::Array bus_array;
            if (!buses.empty()) {
                for (const auto& bus : buses) {
                    json::Node node_bus{std::string(bus)};
                    bus_array.push_back(node_bus);
                }
            }
            response.AsMap()["buses"] = std::move(bus_array);
        }
    } else if (type == "Map") {
        response.AsMap()["map"] = map_json;
    }

    responses.push_back(std::move(response));
}

void ParseBus(const json::Node& node, TransportCatalogue& catalogue) {
    Bus bus;
    bus.name = node.AsMap().at("name").AsString();

    bus.is_roundtrip = node.AsMap().at("is_roundtrip").AsBool();

    const auto& stops = node.AsMap().at("stops").AsArray();
    
    bus.last_elem = catalogue.FindStop(stops.back().AsString());
    
    std::vector<std::string_view> stop_names;
    
    for (const auto& stop_name : stops) {
        stop_names.push_back(stop_name.AsString());
    }

    if (!bus.is_roundtrip) {
        std::vector<std::string_view> reversed_stops(stop_names.rbegin(), stop_names.rend());
        stop_names.insert(stop_names.end(), reversed_stops.begin() + 1, reversed_stops.end());
    }

    for (const auto& stop_name : stop_names) {
        const Stop* stop_ptr = catalogue.FindStop(stop_name);
        if (stop_ptr) {
            bus.stops.push_back(const_cast<Stop*>(stop_ptr));
        }
    }

    catalogue.AddBus(bus);
}
    
void ParseStop(const json::Node& node, TransportCatalogue& catalogue) {
    Stop stop;
    stop.name = node.AsMap().at("name").AsString();
    double latitude = node.AsMap().at("latitude").AsDouble();
    double longitude = node.AsMap().at("longitude").AsDouble();

    stop.coord = geo::Coordinates{latitude, longitude};
    catalogue.AddStop(stop);
}
    
void JsonReader::ReadJson(std::istream& input, TransportCatalogue& catalogue, std::ostream& output) {
    auto doc = json::Load(input);
    const auto& root = doc.GetRoot().AsMap();

    const auto& base_requests = root.at("base_requests").AsArray();
    
    // cначала добавляем все остановки
    for (const auto& request : base_requests) {
        const auto& req_map = request.AsMap();
        if (req_map.at("type").AsString() == "Stop") {
            ParseStop(req_map, catalogue);
        }
    }

    // добавляем расстояния до соседних остановок
    for (const auto& request : base_requests) {
        const auto& req_map = request.AsMap();
        if (req_map.at("type").AsString() == "Stop") {
            const auto& stop_name = req_map.at("name").AsString();
            const auto& road_distances = req_map.at("road_distances").AsMap();
            const Stop* current_stop = catalogue.FindStop(stop_name);
            if (current_stop) {
                for (const auto& [key, value] : road_distances) {
                    catalogue.AddStopsDistance(current_stop, catalogue.FindStop(key), value.AsInt());
                }
            }
        }
    }
    
    // добавляем маршруты
    for (const auto& request : base_requests) {
        const auto& req_map = request.AsMap();
        if (req_map.at("type").AsString() == "Bus") {
            ParseBus(req_map, catalogue);
        }
    }
    
    //обработка render_settings
    RenderSettings render_settings;
    const auto& render_settings_node = root.at("render_settings");
    ProcessRenderSettings(render_settings_node, render_settings);
    MapRenderer renderer;
    std::string map_json = renderer.RenderSvg(render_settings, catalogue);
    
    // обработка stat_request
    const auto& state_requests = root.at("stat_requests").AsArray(); 
    std::vector<json::Node> responses;
    for (const auto& request : state_requests) {
        ProcessStateRequest(request, catalogue, responses, map_json);
    }

    json::Array response_array;
    for (const auto& response : responses) {
        response_array.push_back(response);
    }
    json::Document response_doc{json::Node{std::move(response_array)}};
    json::Print(response_doc, output);
}
    
} // namespace json_reader