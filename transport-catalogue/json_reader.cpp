#include "json_reader.h"
#include <fstream>
#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json_reader.h"
#include "svg.h"


#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

namespace Transport
{
    namespace InputCataloge
    {
       void ApplyInsertJSONCommands([[maybe_unused]] Transport::Data::TransportCatalogue& catalogue, const json::Node& insert_array)
       {
            for (const auto& node /*map_Ы*/ : insert_array.AsArray())
            {
                std::string command = node.AsMap().at("type").AsString();
                if (command != "Stop")
                {
                    continue;
                }

                catalogue.AddStop(node.AsMap().at("name").AsString()
                    , { node.AsMap().at("latitude").AsDouble(), node.AsMap().at("longitude").AsDouble() });

                for (const auto& road_distances : node.AsMap().at("road_distances").AsMap())
                {
                    catalogue.AddDistBetweenStops(node.AsMap().at("name").AsString()
                        , road_distances.first, road_distances.second.AsInt());
                }
            }

            for (const auto& node /*map_Ы*/ : insert_array.AsArray())
            {
                std::string command = node.AsMap().at("type").AsString();
                if (command != "Bus")
                {
                    continue;
                }

                std::vector<std::string_view> route;
                route.reserve(node.AsMap().at("stops").AsArray().size());

                for (const auto& node_whith_stop : node.AsMap().at("stops").AsArray())
                {
                    route.push_back(node_whith_stop.AsString());
                }

                catalogue.AddBus(node.AsMap().at("name").AsString()
                    , route
                    , node.AsMap().at("is_roundtrip").AsBool());
            }
        }

         json::Document ApplyStatJSONCommands(Transport::Data::TransportCatalogue& catalogue, const json::Node& stat_array, const svg::Document& svg_document)
        {
            json::Array result_aray;
            for (const auto& node /*map_Ы*/ : stat_array.AsArray())
            {
                std::string command = node.AsMap().at("type").AsString();

                if (command == "Map")
                {
                    json::Dict current_map;

                    std::stringstream ss;
                    svg_document.Render(ss);
                    current_map.insert(std::map<std::string, std::string>::value_type("map", ss.str()));

                    current_map.insert(std::map<std::string, int>::value_type("request_id", node.AsMap().at("id").AsInt()));
                    result_aray.push_back(std::move(current_map));

                    continue;
                }

                if (command == "Stop")
                {
                    json::Dict current_map;
                    current_map.insert(std::map<std::string, int>::value_type("request_id", node.AsMap().at("id").AsInt()));

                    const auto stop = catalogue.GetStop(node.AsMap().at("name").AsString());
                    if (stop == nullptr)
                    {
                        current_map.insert(std::map<std::string, std::string>::value_type("error_message", "not found"));
                    }
                    else
                    {
                        current_map.insert(std::map<std::string
                                , json::Array>::value_type("buses", json::Array(stop->buses.begin(), stop->buses.end())));
                    }
                    result_aray.push_back(std::move(current_map));
                    continue;
                }

                if (command == "Bus")
                {
                    json::Dict current_map;
                    current_map.insert(std::map<std::string, int>::value_type("request_id", node.AsMap().at("id").AsInt()));

                    const auto bus = catalogue.GetBus(node.AsMap().at("name").AsString());
                    if (bus == nullptr)
                    {
                        current_map.insert(std::map<std::string, std::string>::value_type("error_message", "not found"));
                    }
                    else
                    {
                        size_t bus_stops;
                        if (bus->is_roundtrip)
                        {
                            bus_stops = bus->stops.size();
                        }
                        else
                        {
                            bus_stops = bus->stops.size() * 2 - 1;
                        }

                        current_map.insert(std::map<std::string
                            , double>::value_type("curvature", 1.0 * bus->route_length.street / bus->route_length.geo));
                        current_map.insert(std::map<std::string
                            , int>::value_type("route_length", bus->route_length.street));
                        current_map.insert(std::map<std::string
                            , int>::value_type("stop_count", bus_stops));
                        current_map.insert(std::map<std::string
                            , int>::value_type("unique_stop_count", bus->unique_stop_count));
                    }
                    result_aray.push_back(std::move(current_map));
                    continue;
                }
            }
             return json::Document(std::move(result_aray));
        }

         //    using Color = std::variant<std::string, svg::Rgb, svg::Rgba>;
         //                                         0           1         2     3       4     5      6
         //  using NodeValue = std::variant<std::nullptr_t, std::string, int, double, bool, Array, Dict>;
         svg::Color ReadColor(const json::Node& color)
         {
             svg::Color result;
             switch (color.GetValue().index())
             {
             case 1: return color.AsString();
             case 5:
                 if (color.AsArray().size() == 3)
                 {
                     return svg::Rgb{
                     static_cast<unsigned char>(color.AsArray().at(0).AsInt())
                     ,static_cast<unsigned char>(color.AsArray().at(1).AsInt())
                     ,static_cast<unsigned char>(color.AsArray().at(2).AsInt()) };
                 }
                 else if (color.AsArray().size() == 4)
                 {
                     return svg::Rgba{
                     static_cast<unsigned char>(color.AsArray().at(0).AsInt())
                     ,static_cast<unsigned char>(color.AsArray().at(1).AsInt())
                     ,static_cast<unsigned char>(color.AsArray().at(2).AsInt())
                     ,color.AsArray().at(3).AsDouble() };
                 }
                 else
                 {
                     return svg::NoneColor;
                 }
             default: return svg::NoneColor;
             }
         }

         map_render::RenderSettings ApplySettingsJSON(const json::Node& settings_array)
         {
             map_render::RenderSettings result;

             result.BUS_LABEL_FONT_SIZE = settings_array.AsMap().at("bus_label_font_size").AsInt();
             
             result.BUS_LABEL_OFFSET = { settings_array.AsMap().at("bus_label_offset").AsArray().at(0).AsDouble(),
                                         settings_array.AsMap().at("bus_label_offset").AsArray().at(1).AsDouble() };

             const auto& color_palette = settings_array.AsMap().at("color_palette").AsArray();
             result.COLOR_PALETTE.clear();
             for (const auto& color : color_palette)
             {
                 result.COLOR_PALETTE.push_back(ReadColor(color));
             }

             result.HEIGHT = settings_array.AsMap().at("height").AsDouble();
             result.LINE_WIDTH = settings_array.AsMap().at("line_width").AsDouble();
             result.PADDING = settings_array.AsMap().at("padding").AsDouble();

             result.STOP_LABEL_FONT_SIZE = settings_array.AsMap().at("stop_label_font_size").AsInt();
             result.STOP_LABEL_OFFSET = { settings_array.AsMap().at("stop_label_offset").AsArray().at(0).AsDouble(),
                                         settings_array.AsMap().at("stop_label_offset").AsArray().at(1).AsDouble() };
             result.STOP_RADIUS = settings_array.AsMap().at("stop_radius").AsDouble();

             result.UNDERLAYER_COLOR = ReadColor(settings_array.AsMap().at("underlayer_color"));

             result.UNDERLAYER_WIDTH = settings_array.AsMap().at("underlayer_width").AsDouble();
             result.WIDTH = settings_array.AsMap().at("width").AsDouble();

             return result;
         }


        void Test(std::istream& input, Transport::Data::TransportCatalogue& catalogue)
        {
            auto json_document = json::Load(input);
            auto base_requests = json_document.GetRequests("base_requests");
            ApplyInsertJSONCommands(catalogue, base_requests);
            //catalogue.GetStop("sd");

            auto render_requests = json_document.GetRequests("render_settings");
            map_render::RenderSettings settings = ApplySettingsJSON(render_requests);
            svg::Document svg_document;
            map_render::RenderToDoc(svg_document, catalogue, settings);
            //svg_document.Render(std::cout);


            auto stat_requests = json_document.GetRequests("stat_requests");
            auto ansver_array = ApplyStatJSONCommands(catalogue, stat_requests, svg_document);

          //  auto out_file = std::ofstream("my out.txt");
          //  json::Print(ansver_array, out_file);
            json::Print(ansver_array, std::cout);
        }
    } // namespace InputCataloge
} // namespace Transport