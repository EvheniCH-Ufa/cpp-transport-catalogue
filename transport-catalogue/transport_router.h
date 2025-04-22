#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <optional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>

namespace Transport {
    namespace Router {

        struct RoutingSettings {
            int bus_wait_time = 0;
            double bus_velocity = 0.0;
        };


        struct WaitItem {
            std::string_view stop_name;  // optimizac?
//            std::string stop_name;
            double time = 0.0;
        };

        struct BusItem {
            std::string_view bus_name;  // optimizac?
//            std::string bus_name;
            double time = 0.0;
            int span_count = 0;
        };

        struct RouteInfo {
            double total_time = 0.0;
            std::vector<std::variant<struct WaitItem, struct BusItem>> items;
        };

        struct EdgeInfo {
            Data::Bus* bus = nullptr;
            //            std::string_view bus_name;
//            std::string bus_name;
            int span_count = 0;
        };



        class TransportRouter {
        public:
            TransportRouter(const Data::TransportCatalogue& catalogue, RoutingSettings settings);

            void BuildGraph();
            std::optional<RouteInfo> FindRoute(Data::Stop* from, Data::Stop* to) const;

        private:
            const Data::TransportCatalogue& catalogue_;
            RoutingSettings settings_;

            graph::DirectedWeightedGraph<double> graph_;
            std::unique_ptr<graph::Router<double>> router_;

//            std::unordered_map<std::string, size_t> stop_to_vertex_;
            std::unordered_map<Data::Stop*, size_t> stop_to_vertex_;
            std::vector<Data::Stop*> vertex_to_stop_;
            //            std::vector<std::string> vertex_to_stop_;
//            std::vector<std::string_view> vertex_to_stop_;

            std::vector<EdgeInfo> edges_info_;

            double ComputeTime(double distance) const;
        };

    } // namespace Router
} // namespace Transport
