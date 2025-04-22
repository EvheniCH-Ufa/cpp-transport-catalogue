#include "transport_router.h"

namespace Transport {
    namespace Router {

        TransportRouter::TransportRouter(const Data::TransportCatalogue& catalogue, RoutingSettings settings)
            : catalogue_(catalogue), settings_(settings) {
            BuildGraph();
        }

        void TransportRouter::BuildGraph() {
            // Create vershines for all stops
            const auto& stops = catalogue_.GetAllStops();

            size_t vertex_id = 0;
            for (const auto& stop : stops) {
                stop_to_vertex_[stop] = vertex_id;
                vertex_to_stop_.push_back(stop);
                vertex_id++;
            }

            graph_ = graph::DirectedWeightedGraph<double>(vertex_id);
            edges_info_.reserve(vertex_id * 10);

            // Add rebra for all buses routes
            for (const auto& bus : catalogue_.GetAllBuses()) {
                const auto& stops = bus->stops;

                // For kagdoy pary stops in the route
                for (size_t i = 0; i < stops.size(); ++i) {

                    double distance_tuda  = 0.0;
                    double distance_obrat = 0.0;

                    for (size_t j = i + 1; j < stops.size(); ++j) {
                        // Compute all distance between stops
                        distance_tuda  += catalogue_.GetDistBetweenStops(stops[j - 1]->name, stops[j]->name);
                        distance_obrat += catalogue_.GetDistBetweenStops(stops[j]->name, stops[j-1]->name);

                        // Total time = wait time + put' time
                        double time = settings_.bus_wait_time + ComputeTime(distance_tuda);
                        int span_count = j - i;

                        // Add rebro
                        size_t from_vertex = stop_to_vertex_.at(stops[i]);
                        size_t to_vertex = stop_to_vertex_.at(stops[j]);
                        graph_.AddEdge({ from_vertex, to_vertex, time });
                        edges_info_.push_back({ bus, span_count });

                        // For non-round buses add rebra obratno
                        if (!bus->is_roundtrip) {
                            time = settings_.bus_wait_time + ComputeTime(distance_obrat);
                            graph_.AddEdge({ to_vertex, from_vertex, time });
                            edges_info_.push_back({ bus, span_count });
                        }
                    }
                }
            }

            router_ = std::make_unique<graph::Router<double>>(graph_);
        }

        double TransportRouter::ComputeTime(double distance) const {
            return distance / (settings_.bus_velocity * 1000.0 / 60.0); // km/h to m/min
        }

        std::optional<RouteInfo> TransportRouter::FindRoute(Data::Stop* from, Data::Stop* to) const {
            if (stop_to_vertex_.count(from) == 0 || stop_to_vertex_.count(to) == 0) {
                return std::nullopt;
            }

            size_t from_vertex = stop_to_vertex_.at(from);
            size_t to_vertex = stop_to_vertex_.at(to);

            auto route_info = router_->BuildRoute(from_vertex, to_vertex);
            if (!route_info) {
                return std::nullopt;
            }

            RouteInfo result;
            result.total_time = route_info->weight;

            for (const auto edge_id : route_info->edges) {
                const auto& edge = graph_.GetEdge(edge_id);
                const auto& edge_info = edges_info_[edge_id];

                // Add wait item
                WaitItem wait;
                wait.stop_name = vertex_to_stop_[edge.from]->name;
                wait.time = settings_.bus_wait_time;
                result.items.push_back(wait);

                // Add bus item
                BusItem bus;
                bus.bus_name = edge_info.bus->name;
                bus.time = edge.weight - settings_.bus_wait_time; // Subtract wait time
                bus.span_count = edge_info.span_count;
                result.items.push_back(bus);
            }
            return result;
        }
    } // namespace Router
} // namespace Transport
