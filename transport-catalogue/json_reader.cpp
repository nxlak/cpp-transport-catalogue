#include "json_reader.h"
#include "transport_router.h"

namespace json_reader {

void ParseColor(const json::Node& color_node, svg::Color& color) {
    if (color_node.IsArray()) {
        if (color_node.AsArray().size() == 3) {
            color = svg::Rgb(
                static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color_node.AsArray().at(2).AsInt())
            );
        } else if (color_node.AsArray().size() == 4) {
            color = svg::Rgba(
                static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color_node.AsArray().at(2).AsInt()),
                color_node.AsArray().at(3).AsDouble()
            );
        }
    } else if (color_node.IsString()) {
        color = color_node.AsString();
    }
}

void JsonReader::ProcessRenderSettings(const json::Node& node, map_renderer::RenderSettings& settings) {
    settings.width = node.AsDict().at("width").AsDouble();
    settings.height = node.AsDict().at("height").AsDouble();
    settings.padding = node.AsDict().at("padding").AsDouble();
    
    settings.line_width = node.AsDict().at("line_width").AsDouble();
    
    settings.stop_radius = node.AsDict().at("stop_radius").AsDouble();
    
    settings.bus_label_font_size = node.AsDict().at("bus_label_font_size").AsInt();
    
    settings.bus_label_offset = {node.AsDict().at("bus_label_offset").AsArray().at(0).AsDouble(), 
                                 node.AsDict().at("bus_label_offset").AsArray().at(1).AsDouble()};
    
    settings.stop_label_font_size = node.AsDict().at("stop_label_font_size").AsInt();
    
    settings.stop_label_offset = {node.AsDict().at("stop_label_offset").AsArray().at(0).AsDouble(), 
                                  node.AsDict().at("stop_label_offset").AsArray().at(1).AsDouble()};

    json::Node underlayer_color_node = node.AsDict().at("underlayer_color");
    
    svg::Color underlayer_color;
    
    ParseColor(underlayer_color_node, underlayer_color);
    
    settings.underlayer_color = underlayer_color;

    settings.underlayer_width = node.AsDict().at("underlayer_width").AsDouble();

    json::Node color_palette_node = node.AsDict().at("color_palette");
    
    for (const auto& color_node : color_palette_node.AsArray()) {
        svg::Color color;
        
        ParseColor(color_node, color);
        
        settings.color_palette.push_back(color);
    }
}

void JsonReader::ProcessStateRequest(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue, std::string map_json, json::Builder& response_array, const transport_catalogue::TransportRouter& router) {
   const auto& type = node.AsDict().at("type").AsString();
   const auto& id = node.AsDict().at("id").AsInt();

   response_array.StartDict().Key("request_id").Value(id);

   if (type == "Bus") {
       const auto& bus_name = node.AsDict().at("name").AsString();

       auto bus_info = catalogue.GetBusInfo(bus_name);
       if (bus_info) {
           response_array.Key("curvature").Value(bus_info->curvature)
                         .Key("route_length").Value(bus_info->distance)
                         .Key("stop_count").Value(bus_info->stops_count)
                         .Key("unique_stop_count").Value(bus_info->unique_stops_count);
       } else {
           response_array.Key("error_message").Value("not found");
       }
   } else if (type == "Stop") {
       const auto& stop_name = node.AsDict().at("name").AsString();
       const transport_catalogue::Stop* stop_ptr = catalogue.FindStop(stop_name);
       if (!stop_ptr) {
           response_array.Key("error_message").Value("not found");
       } else {
           auto buses = catalogue.GetBusesByStop(stop_name);
           json::Array bus_array;
           if (!buses.empty()) {
               for (const auto& bus : buses) {
                   bus_array.push_back(std::string(bus));
               }
           }
           response_array.Key("buses").Value(std::move(bus_array));
       }
   } else if (type == "Map") {
       response_array.Key("map").Value(map_json);
   } else if (type == "Route") {
        const auto& from_stop = node.AsDict().at("from").AsString();
        const auto& to_stop = node.AsDict().at("to").AsString();

        if (!catalogue.FindStop(from_stop) || !catalogue.FindStop(to_stop)) {
            response_array.Key("error_message").Value("not found");
        } else {
            auto route_info = router.FindRoute(from_stop, to_stop);
            if (!route_info) {
                response_array.Key("error_message").Value("not found");
            } else {
                response_array.Key("total_time").Value(route_info->total_time); 
                response_array.Key("items").StartArray();
                for (const auto& item : route_info->items) {
                    response_array.StartDict()
                        .Key("type").Value(item.type == transport_catalogue::RouteItem::ItemType::Wait ? "Wait" : "Bus");
                    if (item.type == transport_catalogue::RouteItem::ItemType::Wait) {
                        response_array.Key("stop_name").Value(item.name);
                    } else {
                        response_array.Key("bus").Value(item.name)
                                      .Key("span_count").Value(static_cast<int>(item.span_count));
                    }
                    response_array.Key("time").Value(item.time);
                    response_array.EndDict();
                }
                response_array.EndArray();
            }
        }
    }

   response_array.EndDict();
}

void ParseBus(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue) {
   transport_catalogue::Bus bus;
   bus.name = node.AsDict().at("name").AsString();

   bus.is_roundtrip = node.AsDict().at("is_roundtrip").AsBool();

   const auto& stops = node.AsDict().at("stops").AsArray();

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
       const transport_catalogue::Stop* stop_ptr = catalogue.FindStop(stop_name);
       if (stop_ptr) {
           bus.stops.push_back(stop_ptr);
       }
   }

   catalogue.AddBus(bus);
}

void ParseStop(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue) {
   transport_catalogue::Stop stop;
   stop.name = node.AsDict().at("name").AsString();
   
   double latitude = node.AsDict().at("latitude").AsDouble();
   double longitude = node.AsDict().at("longitude").AsDouble();

   stop.coord = geo::Coordinates{latitude, longitude};
   
   catalogue.AddStop(stop);
}

void JsonReader::ReadJson(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, std::ostream& output) {
   auto doc = json::Load(input);
   const auto& root = doc.GetRoot().AsDict();

   const auto& base_requests = root.at("base_requests").AsArray();

   // First add all stops
   for (const auto& request : base_requests) {
       const auto& req_map = request.AsDict();
       if (req_map.at("type").AsString() == "Stop") {
           ParseStop(req_map, catalogue);
       }
   }

   // Add distances to neighboring stops
   for (const auto& request : base_requests) {
       const auto& req_map = request.AsDict();
       if (req_map.at("type").AsString() == "Stop") {
           const auto& stop_name = req_map.at("name").AsString();
           const auto& road_distances = req_map.at("road_distances").AsDict();
           const transport_catalogue::Stop* current_stop = catalogue.FindStop(stop_name);
           if (current_stop) {
               for (const auto& [key, value] : road_distances) {
                   catalogue.AddStopsDistance(current_stop, catalogue.FindStop(key), value.AsInt());
               }
           }
       }
   }

   // Add routes
   for (const auto& request : base_requests) {
       const auto& req_map = request.AsDict();
       if (req_map.at("type").AsString() == "Bus") {
           ParseBus(req_map, catalogue);
       }
   }

   // Process routing settings
   const auto& routing_settings = root.at("routing_settings").AsDict();
   
   int bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
   
   double bus_velocity = routing_settings.at("bus_velocity").AsDouble();

   transport_catalogue::TransportRouter router(bus_wait_time, bus_velocity);
   
   router.BuildGraph(catalogue);

   // Process render settings
   map_renderer::RenderSettings render_settings;
   
   const auto& render_settings_node = root.at("render_settings");
   
   ProcessRenderSettings(render_settings_node, render_settings);

   map_renderer::MapRenderer renderer;
   
   std::string map_json = renderer.RenderSvg(render_settings, catalogue);

   // Process state requests
   json::Builder builder;
   
   auto response_array = builder.StartArray();
   
   const auto& state_requests = root.at("stat_requests").AsArray(); 
   
   for (const auto& request : state_requests) {
       ProcessStateRequest(request, catalogue, map_json, builder, router);
   }
   
   response_array.EndArray();

   json::Print(json::Document{builder.Build()}, output);
}    

} // namespace json_reader