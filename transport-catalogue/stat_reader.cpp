#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
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
                    output << (*find_bus).stops.size() << " stops on route, "s
                        << (*find_bus).unique_stop_count << " unique stops, "s
                        << std::setprecision(6) << (*find_bus).route_length << " route length\n"s;
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
                    const auto& buses_this_stop = find_stop->GetBuses();
                    if (buses_this_stop.empty())
                    {
                        output << "no buses\n"s;
                        return;
                    }
                    
                    output << "buses"s;
                    for (const auto& bus_this_stop : buses_this_stop)
                    {
                        output << " "s << bus_this_stop;
                    }
                    output << "\n"s;
                }
                return;
            }
        }

        void Test(std::istream& input, std::ostream& output, Transport::Data::TransportCatalogue& catalogue) // (std::istream&, std::ostream&)
        {
            int stat_request_count;
            input >> stat_request_count >> std::ws;
            for (int i = 0; i < stat_request_count; ++i) {
                std::string line;
                std::getline(input, line);
                Transport::PrintCataloge::ParseAndPrintStat(catalogue, line, output);
            }
        }
    }
}
