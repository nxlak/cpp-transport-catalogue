#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.emplace_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}
    
void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.emplace_back(bus);
    busname_to_bus_[buses_.back().name] = &buses_.back();
    
    for (const auto& stop : buses_.back().stops) {
        stop_to_buses_[stop].emplace(buses_.back().name);
    }
}
    
const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

double TransportCatalogue::CalculateRouteLength(const std::vector<std::string>& stop_names) const {
    double total_length = 0.0;
        
    for (size_t i = 0; i < stop_names.size() - 1; ++i) {
        const Stop* from_stop = FindStop(stop_names[i]);
        const Stop* to_stop = FindStop(stop_names[i + 1]);
        if (from_stop && to_stop) {
            total_length += GetStopsDistance(from_stop, to_stop);
        }
    }
   return total_length;
}

double TransportCatalogue::CalculateGeoLength(const std::vector<std::string>& stop_names) const {
    double total_length = 0.0;
        
    for (size_t i = 0; i < stop_names.size() - 1; ++i) {
        const Stop* from_stop = FindStop(stop_names[i]);
        const Stop* to_stop = FindStop(stop_names[i + 1]);
        if (from_stop && to_stop) {
            total_length += geo::ComputeDistance(from_stop->coord, to_stop->coord);
        }
    }

   return total_length;
}
    
std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
    const Bus* bus = FindBus(bus_name);
    if (!bus) {
        return std::nullopt; 
    }

    BusInfo info = {0, 0, 0.0, 0.0};
    info.stops_count = bus->stops.size();
    std::unordered_set<Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    info.unique_stops_count = unique_stops.size();

    if (!bus->stops.empty()) {
        std::vector<std::string> stop_names;
        for (const auto& stop : bus->stops) {
            stop_names.push_back(stop->name);
        }
        double route = CalculateRouteLength(stop_names);
        info.distance = route;
        double geo = CalculateGeoLength(stop_names);
        info.curvature = route / geo;
    }
    return info;   
}

std::vector<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
    const Stop* stop = FindStop(stop_name);
    std::vector<std::string_view> res;
    if (stop) {
        auto it = stop_to_buses_.find(stop);
        if (it != stop_to_buses_.end()) {
            for (const auto& bus : it->second) {
                res.push_back(bus);
            }
        }
    }
    std::sort(res.begin(), res.end());
    return res;
}

void TransportCatalogue::AddStopsDistance(const Stop* from, const Stop* to, int di) {
    stops_di_[{from, to}] = di;
    if (stops_di_.find({to, from}) == stops_di_.end()) {
        stops_di_[{to, from}] = di;
    }
}
    
int TransportCatalogue::GetStopsDistance(const Stop* from, const Stop* to) const {
    auto it = stops_di_.find({from, to});
    if (it != stops_di_.end()) {
        return it->second;
    }
    it = stops_di_.find({to, from});
    if (it != stops_di_.end()) {
        return it->second;
    }
    return 0;
}    
    
} //namespace transport_catalogue 
