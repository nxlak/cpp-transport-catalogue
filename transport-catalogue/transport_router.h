#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <map>
#include <memory>
#include <string_view>

namespace transport_catalogue {

class TransportRouter {
public:
    TransportRouter(int bus_wait_time = 0, double bus_velocity = 0.0);

    void BuildGraph(const TransportCatalogue& catalogue);
    std::optional<graph::Router<double>::RouteInfo> FindRoute(std::string_view stop_from, std::string_view stop_to) const;

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    void InitializeStops(const TransportCatalogue& catalogue);
    void AddBusEdges(const TransportCatalogue& catalogue);

    int bus_wait_time_;
    double bus_velocity_;

    graph::DirectedWeightedGraph<double> graph_;
    std::map<std::string_view, graph::VertexId> stop_ids_;
    std::unique_ptr<graph::Router<double>> router_;
};

} // namespace transport_catalogue