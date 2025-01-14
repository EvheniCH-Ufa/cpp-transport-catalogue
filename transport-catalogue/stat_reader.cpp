#include "stat_reader.h"
#include "transport_catalogue.h"

#include <ostream>
#include <iomanip>

namespace Transport
{
    namespace PrintCataloge
    {
        void ParseAndPrintStat(const Transport::Data::TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            using namespace std::literals;

            auto space = request.find(' ');
            auto request_object_type = request.substr(0, space);
            auto request_object_name = request.substr(space + 1, request.size() - space);

            if (request_object_type == "Bus")
            {
                const Transport::Data::Bus* find_bus = tansport_catalogue.GetBus(request_object_name);
                output << "Bus "s << request_object_name << ": "s;
                if (find_bus == nullptr)
                {
                    output << "not found\n"s;
                }
                else
                {
                    output << std::to_string((*find_bus).stops.size()) << " stops on route, "s
                        << std::to_string((*find_bus).unique_stop_count) << " unique stops, "s
                        << std::setprecision(6) << std::to_string((*find_bus).route_length) << " route length\n"s;
                }
                return;
            }

            if (request_object_type == "Stop")
            {
                const Transport::Data::Stop* find_stop = tansport_catalogue.GetStop(request_object_name);
                output << "Stop "s << request_object_name << ": "s;
                if (find_stop == nullptr)
                {
                    output << "not found\n"s;
                }
                else
                {
                    auto buses_this_stop = find_stop->GetBuses();
                    if (buses_this_stop.empty())
                    {
                        output << "no buses\n"s;
                        return;
                    }

                    std::vector<Transport::Data::Bus*> buses_for_print(buses_this_stop.begin(), buses_this_stop.end());

                    std::sort(buses_for_print.begin(), buses_for_print.end(),
                        [](const Transport::Data::Bus* left, const Transport::Data::Bus* right)
                        {
                            return (left->name < right->name);
                        });

                    output << "buses"s;
                    for (const auto& bus_this_stop : buses_for_print)
                    {
                        output << " "s << bus_this_stop->name;
                    }
                    output << "\n"s;
                }
                return;
            }
        }
    }
}
