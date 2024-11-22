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
    
    geo::Coordinates GetCoordinates() const {
        return coord;
    }
};
    
struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
    const Stop* last_elem; 
};
    
struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double distance;
    double curvature;
};
    
struct StopsHasher {
public:    
    size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
        return s_hasher(stops.first->name) + s_hasher(stops.second->name) * 37;
    }
    
    std::hash<std::string> s_hasher;
};
    
class TransportCatalogue {
public:
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    const Stop* FindStop(std::string_view name) const;
    const Bus* FindBus(std::string_view name) const;
    std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const;
    double CalculateRouteLength(const std::vector<std::string>& stop_names) const;
    double CalculateGeoLength(const std::vector<std::string>& stop_names) const;
    std::vector<std::string_view> GetBusesByStop(std::string_view stop_name) const;
    void AddStopsDistance(const Stop* from, const Stop* to, int di);
    int GetStopsDistance(const Stop* from, const Stop* to) const;
    std::deque<Bus> GetBuses() const {
        return buses_;
    }
    std::deque<Stop> GetStops() const {
        return stops_;
    }
    const std::unordered_map<std::string_view, Stop*>& GetStopNameToStopMap() const {
        return stopname_to_stop_;
    }

    const std::unordered_map<std::string_view, Bus*>& GetBusNameToBusMap() const {
        return busname_to_bus_;
    }
private:
    std::deque<Stop> stops_; 
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::unordered_set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopsHasher> stops_di_;
};

} //namespace transport_catalogue