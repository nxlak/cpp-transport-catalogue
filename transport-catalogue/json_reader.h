#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"
#include <sstream>

namespace json_reader {

using namespace json;
using namespace transport_catalogue;
using namespace map_renderer;
    
class JsonReader {
public:
    void ProcessRenderSettings(const json::Node& node, RenderSettings& settings);
    void ProcessStateRequest(const json::Node& node, TransportCatalogue& catalogue, std::vector<json::Node>& responses, std::string map_json);
    void ReadJson(std::istream& input, TransportCatalogue& catalogue, std::ostream& output);
};
    
} // namespace json_reader