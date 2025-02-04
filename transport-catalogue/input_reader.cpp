#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

namespace Transport
{
    namespace InputCataloge
    {
        /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
        Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }


        struct StopCoordinateAndDistance {
            std::string_view Coordinate;
            std::string_view Distance;
        };


        // Parse  55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
        // to     55.574371, 37.6517
        // and    7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
        StopCoordinateAndDistance ParseStopDescription(std::string_view str) {
            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');        //find first ','
            comma = str.find(',', comma + 1);  //find second ','

            if (comma == str.npos) {
                return { str, str.substr(0,0) };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);
            return { str.substr(not_space, comma - not_space), str.substr(not_space2) };
        }

        // Parse  "7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"
        // to {{"current", "Rossoshanskaya ulitsa", 7500}, {"current", "Biryusinka", 1800}, {"current", "Universam", 2400}}
        void ParseLenToStopDescription(Transport::Data::TransportCatalogue& catalogue, std::string_view current_stop, std::string_view str) {
            if (str.empty())
            {
                return;
            }

            size_t base = 0;
            while (true)
            {
                auto not_space = str.find_first_not_of(' ', base);
                auto m_char_pos = str.find('m', not_space);
                auto to_char_pos = str.find("to ", m_char_pos);
                auto not_space2 = str.find_first_not_of(' ', to_char_pos + 3);
                auto comma = str.find(',', not_space2 + 1);

                catalogue.AddDistBetweenStops(current_stop, str.substr(not_space2, comma - not_space2), std::stoi(std::string(str.substr(not_space, m_char_pos - not_space))));
                if (comma == std::string::npos)
                {
                    break;
                }
                base = comma + 1;
            }
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),  // command
                    std::string(line.substr(not_space, colon_pos - not_space)), // id bus or stop
                    std::string(line.substr(colon_pos + 1)) };  // description
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::ApplyCommands([[maybe_unused]] Transport::Data::TransportCatalogue& catalogue) const {
            // Реализуйте метод самостоятельно
            // first - commands whitch add stops

            std::vector<std::tuple<std::string_view, std::string_view, int>> lengths_for_write;
            lengths_for_write.reserve(commands_.size());

            for (auto& command : commands_)
            {
                if (command.command != "Stop")
                {
                    continue;
                }
                
                auto [coordinates_description, len_to_stops_description] = ParseStopDescription(command.description);
                catalogue.AddStop(command.id, ParseCoordinates(coordinates_description));
                ParseLenToStopDescription(catalogue, command.id, len_to_stops_description);  
            }

            // second - commands whitch add buses
            for (auto& command : commands_)
            {
                if (command.command != "Bus")
                {
                    continue;
                }
                std::vector<std::string_view> route = ParseRoute(command.description);
                catalogue.AddBus(command.id, route);
            }
        }

        void Test(std::istream& input, Transport::Data::TransportCatalogue& catalogue)
        {
            int base_request_count;
            input >> base_request_count >> std::ws;
            InputReader reader;

            for (int i = 0; i < base_request_count; ++i)
            {
                std::string line;
                std::getline(input, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
    }
}
