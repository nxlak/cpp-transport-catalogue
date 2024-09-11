#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <utility>
#include "algorithm"

#include "geo.h"

namespace transport_catalogue {

class TransportCatalogue {
public:
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
    
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    const Stop* FindStop(const std::string_view name) const;
    const Bus* FindBus(const std::string_view name) const;
    BusInfo GetBusInfo(const std::string_view bus_name) const;
    double CalculateRouteLength(const std::vector<std::string>& stop_names) const;
    std::vector<std::string> GetBusesByStop(const std::string_view stop_name) const;
    
private:
    std::deque<Stop> stops_; 
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::unordered_set<std::string>> stop_to_buses_;
};

} //namespace transport_catalogue
