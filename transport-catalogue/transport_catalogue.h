#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <utility>
#include "algorithm"
#include <optional>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coord;
};
    
struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};
    
struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double distance;
};
    
class TransportCatalogue {
public:

    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    const Stop* FindStop(std::string_view name) const;
    const Bus* FindBus(std::string_view name) const;
    std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const;
    double CalculateRouteLength(const std::vector<std::string>& stop_names) const;
    std::vector<std::string_view> GetBusesByStop(std::string_view stop_name) const;
    
private:
    std::deque<Stop> stops_; 
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::unordered_set<std::string_view>> stop_to_buses_;
};

} //namespace transport_catalogue
