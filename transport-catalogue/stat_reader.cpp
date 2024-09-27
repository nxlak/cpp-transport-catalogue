#include "stat_reader.h"

#include <iostream>
#include <string>
#include <string_view>

namespace transport_catalogue {
namespace stat {

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    if (request.starts_with("Bus ")) {
        std::string_view prefix = "Bus ";
        request.remove_prefix(prefix.size());
        
        auto info = transport_catalogue.GetBusInfo(request);
        if (info.has_value()) {
            const BusInfo& bus_info = info.value();
            output << "Bus " << request << ": " 
                   << bus_info.stops_count << " stops on route, "
                   << bus_info.unique_stops_count << " unique stops, " 
                   << bus_info.distance << " route length, " 
                   << bus_info.curvature << " curvature\n";
        }
        else {
            output << "Bus " << request << ": not found\n"; 
        }
    } else if (request.starts_with("Stop ")) {
        std::string_view prefix = "Stop ";
        request.remove_prefix(prefix.size());
        
        const auto& buses = transport_catalogue.GetBusesByStop(request);
        
        if (buses.empty()) {
            const Stop* stop = transport_catalogue.FindStop(request);
            if (!stop) {
                output << "Stop " << request << ": not found\n";
            } else {
                output << "Stop " << request << ": no buses\n";
            }
        } else {
            output << "Stop " << request << ": buses ";
            for (const auto& bus : buses) {
                output << bus << " ";
            }
            output << "\n";
        }
    }
}
    
} //namespace stat
} //namespace transport_catalogue 
