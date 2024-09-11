#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.emplace_back(std::move(stop));
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.emplace_back(std::move(bus));
    busname_to_bus_[buses_.back().name] = &buses_.back();
    
    for (const auto& stop : bus.stops) {
        stop_to_buses_[stop].emplace(bus.name);
    }
}

const TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string_view name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string_view name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

double TransportCatalogue::CalculateRouteLength(const std::vector<std::string>& stop_names) const {
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

TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const std::string_view bus_name) const {
    BusInfo info = {0, 0, 0.0}; 
    const Bus* bus = FindBus(bus_name);
    if (bus) {
        info.stops_count = bus->stops.size();
        
        std::unordered_set<Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
        info.unique_stops_count = unique_stops.size();
        
        if (!bus->stops.empty()) {
            std::vector<std::string> stop_names;
            for (const auto& stop : bus->stops) {
                stop_names.push_back(stop->name);
            }
            info.distance = CalculateRouteLength(stop_names);
        }
    }
    return info;    
}

std::vector<std::string> TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
    const Stop* stop = FindStop(stop_name);
    std::vector<std::string> res;
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
    
} //namespace transport_catalogue 
