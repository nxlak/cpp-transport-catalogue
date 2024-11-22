#include "transport_router.h"

namespace transport_catalogue {

TransportRouter::TransportRouter(int bus_wait_time, double bus_velocity)
    : bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {
}

void TransportRouter::BuildGraph(const TransportCatalogue& catalogue) {
    InitializeStops(catalogue);
    AddBusEdges(catalogue);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

void TransportRouter::InitializeStops(const TransportCatalogue& catalogue) {
    const auto& stops = catalogue.GetStopNameToStopMap();
    size_t vertex_count = stops.size() * 2;
    graph_ = graph::DirectedWeightedGraph<double>(vertex_count);

    graph::VertexId vertex_id = 0;
    for (const auto& [stop_name, stop_info] : stops) {
        stop_ids_[stop_name] = vertex_id;

        graph_.AddEdge(graph::Edge<double>{
            stop_info->name,     
            0,                         
            vertex_id,                       
            vertex_id + 1,                    
            static_cast<double>(bus_wait_time_) 
        });

        vertex_id += 2;
    }
}

void TransportRouter::AddBusEdges(const TransportCatalogue& catalogue) {
    const auto& buses = catalogue.GetBusNameToBusMap();

    for (const auto& [bus_name, bus_info] : buses) {
        const auto& stops = bus_info->stops;
        size_t stop_count = stops.size();

        for (size_t i = 0; i < stop_count; ++i) {
            for (size_t j = i + 1; j < stop_count; ++j) {
                const Stop* stop_from = stops[i];
                const Stop* stop_to = stops[j];

                double total_distance_forward = 0.0;
                double total_distance_backward = 0.0;

                for (size_t k = i + 1; k <= j; ++k) {
                    total_distance_forward += catalogue.GetStopsDistance(stops[k - 1], stops[k]);
                }

                for (size_t k = j; k > i; --k) {
                    total_distance_backward += catalogue.GetStopsDistance(stops[k], stops[k - 1]);
                }

                size_t span_count = j - i;

                double travel_time_forward = total_distance_forward / (bus_velocity_ * (1000.0 / 60.0));

                graph_.AddEdge(graph::Edge<double>{
                    bus_info->name,
                    span_count,
                    stop_ids_.at(stop_from->name) + 1,
                    stop_ids_.at(stop_to->name),
                    travel_time_forward
                });

                if (!bus_info->is_roundtrip) {
                    double travel_time_backward = total_distance_backward / (bus_velocity_ * (1000.0 / 60.0));

                    graph_.AddEdge(graph::Edge<double>{
                        bus_info->name,
                        span_count,
                        stop_ids_.at(stop_to->name) + 1,
                        stop_ids_.at(stop_from->name),
                        travel_time_backward
                    });
                }
            }
        }
    }
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::FindRoute(std::string_view stop_from, std::string_view stop_to) const {
    auto from_it = stop_ids_.find(stop_from);
    auto to_it = stop_ids_.find(stop_to);

    if (from_it == stop_ids_.end() || to_it == stop_ids_.end()) {
        return std::nullopt;
    }

    return router_->BuildRoute(from_it->second, to_it->second);
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return graph_;
}

} // namespace transport_catalogue