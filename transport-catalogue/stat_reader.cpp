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
        const TransportCatalogue::Bus* bus = transport_catalogue.FindBus(request);

        if (bus) {
            TransportCatalogue::BusInfo info = transport_catalogue.GetBusInfo(request);
            
            output << "Bus " << request << ": " 
                   << info.stops_count << " stops on route, "
                   << info.unique_stops_count << " unique stops, " 
                   << info.distance << " route length\n";
            
        } else {
            output << "Bus " << request << ": not found\n"; 
        }
    } else if (request.starts_with("Stop ")) {
        std::string_view prefix = "Stop ";
        request.remove_prefix(prefix.size());
        
        const auto& buses = transport_catalogue.GetBusesByStop(request);
        
        if (buses.empty()) {
            const TransportCatalogue::Stop* stop = transport_catalogue.FindStop(request);
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
