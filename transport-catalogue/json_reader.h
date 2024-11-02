#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"
#include <sstream>
#include "json_builder.h"

namespace json_reader {

class JsonReader {
public:
    void ProcessRenderSettings(const json::Node& node, map_renderer::RenderSettings& settings);
    //void ProcessStateRequest(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue, std::vector<json::Node>& responses, std::string map_json);
    void ProcessStateRequest(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue, std::string map_json, json::Builder& response_array);
    void ReadJson(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, std::ostream& output);
};
    
} // namespace json_reader