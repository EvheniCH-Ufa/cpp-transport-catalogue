#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>

#include "svg.h"

namespace Transport
{
        void JsonReader::ApplyInsertJSONCommands(const json::Node& insert_array)
        {
            for (const auto& node /*map_Ы*/ : insert_array.AsArray())
            {
                std::string command = node.AsMap().at("type").AsString();
                if (command != "Stop")
                {
                    continue;
                }

                catalogue_.AddStop(node.AsMap().at("name").AsString()
                    , { node.AsMap().at("latitude").AsDouble(), node.AsMap().at("longitude").AsDouble() });

                for (const auto& road_distances : node.AsMap().at("road_distances").AsMap())
                {
                    catalogue_.AddDistBetweenStops(node.AsMap().at("name").AsString()
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

                catalogue_.AddBus(node.AsMap().at("name").AsString()
                    , route
                    , node.AsMap().at("is_roundtrip").AsBool());
            }
        }

        json::Document JsonReader::ApplyStatJSONCommandsBuild(const json::Node& stat_array, const svg::Document& svg_document)
        {
            {
                json::Builder builder;

                json::ArrayItemContext build_array_item_context = builder.StartArray();
                json::Array result_aray;

                for (const auto& node /*map_Ы*/ : stat_array.AsArray())
                {

                    std::string command = node.AsMap().at("type").AsString();
                    if (command == "Map")
                    {
                        // тут фактически МАП с двумя записями: {"map" + сама карта} ,  {"request_id" + id }
                        std::stringstream ss;
                        svg_document.Render(ss);

                        build_array_item_context.StartDict()
                            .Key("map").Value(ss.str())
                            .Key("request_id").Value(node.AsMap().at("id").AsInt())
                            .EndDict();

                        continue;
                    }

                    if (command == "Stop")
                    {
                        auto current_map_builder = build_array_item_context.StartDict();
                            
                        current_map_builder.Key("request_id").Value(node.AsMap().at("id").AsInt());
                        const auto stop = catalogue_.GetStop(node.AsMap().at("name").AsString());
                        if (stop == nullptr)
                        {
                            current_map_builder.Key("error_message").Value("not found"); 
                        }
                        else
                        {
                            auto buses_array_builder = current_map_builder.Key("buses").StartArray();
                            for (auto& bus : stop->buses)
                            {
                                buses_array_builder.Value(bus);
                            }
                            buses_array_builder.EndArray();
                        }
                        current_map_builder.EndDict();
                        continue;
                    }

                    if (command == "Bus")
                    {
                        auto current_map_builder = build_array_item_context.StartDict();
                        current_map_builder.Key("request_id").Value(node.AsMap().at("id").AsInt());

                        json::Dict current_map;

                        const auto bus = catalogue_.GetBus(node.AsMap().at("name").AsString());
                        if (bus == nullptr)
                        {
                            current_map_builder.Key("error_message").Value("not found");
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
                            current_map_builder.Key("curvature").Value(1.0 * bus->route_length.street / bus->route_length.geo);
                            current_map_builder.Key("route_length").Value(bus->route_length.street);
                            current_map_builder.Key("stop_count").Value((int)bus_stops);
                            current_map_builder.Key("unique_stop_count").Value((int)bus->unique_stop_count);
                        }
                        current_map_builder.EndDict();

                        continue;
                    }
                }
                return json::Document(build_array_item_context.EndArray().Build());
            }
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

        map_render::RenderSettings JsonReader::ApplySettingsJSON(const json::Node& settings_array)
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

        void JsonReader::ReadJson(std::istream& input)
        {
            auto json_document = json::Load(input);
            auto base_requests = json_document.GetRequests("base_requests");
            ApplyInsertJSONCommands(base_requests);

            auto render_requests = json_document.GetRequests("render_settings");
            map_render::RenderSettings settings = ApplySettingsJSON(render_requests);

            svg::Document svg_document;
            map_render::MapRenderer map_renderer(catalogue_, settings);
            map_renderer.RenderToSVGDoc(svg_document);

            auto stat_requests = json_document.GetRequests("stat_requests");
            auto ansver_array = ApplyStatJSONCommandsBuild(stat_requests, svg_document);

          //  auto out_file = std::ofstream("my out.txt"); // для отладки
          //  json::Print(ansver_array, out_file);         // для отладки
            json::Print(ansver_array, std::cout);
        }
} // namespace Transport