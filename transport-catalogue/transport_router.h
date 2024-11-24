#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include <map>
#include <memory>
#include <string_view>
#include <optional>
#include <vector>

namespace transport_catalogue {

struct RouteResult {
    double total_time; 
    std::vector<graph::EdgeId> edges; 
};

class TransportRouter {
public:
    TransportRouter(int bus_wait_time = 0, double bus_velocity = 0.0);

    void BuildGraph(const TransportCatalogue& catalogue);
    std::optional<RouteResult> FindRoute(std::string_view stop_from, std::string_view stop_to) const;

    const graph::Edge<double>& GetEdge(graph::EdgeId edge_id) const; 

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