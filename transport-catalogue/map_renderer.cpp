#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 */
namespace map_render {
    const SphereProjector MakeSphereProjector(const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings)
    {
        std::vector<geo::Coordinates> geo_coords;
        auto all_buses = catalog.GetAllBuses();
        for (const auto& bus : all_buses)
        {
            for (const auto& stop : bus->stops)
            {
                geo_coords.push_back(stop->coordinates);
            }
        }
        return SphereProjector{geo_coords.begin(), geo_coords.end(), settings.WIDTH, settings.HEIGHT, settings.PADDING };
    }
    

    std::pair<svg::Text, svg::Text> MakeBusLabel(const RenderSettings& settings, const svg::Color& color, const svg::Point& pos, const std::string& text)
    {
        svg::Text result_back, result_text;

        result_back.SetPosition(pos);
        result_back.SetOffset({ settings.BUS_LABEL_OFFSET.x, settings.BUS_LABEL_OFFSET.y });
        result_back.SetFontSize(settings.BUS_LABEL_FONT_SIZE);
        result_back.SetFontFamily("Verdana");
        result_back.SetFontWeight("bold");
        result_back.SetData(text);

        result_text = result_back;

        result_back.SetFillColor(settings.UNDERLAYER_COLOR);
        result_back.SetStrokeColor(settings.UNDERLAYER_COLOR);
        result_back.SetStrokeWidth(settings.UNDERLAYER_WIDTH);
        result_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        result_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        result_text.SetFillColor(color);

        return { result_back, result_text };
    }

    std::pair<svg::Text, svg::Text> MakeStopLabel(const RenderSettings& settings, const svg::Point& pos, const std::string& text)
    {
        svg::Text result_back, result_text;

        result_back.SetPosition(pos);
        result_back.SetOffset({ settings.STOP_LABEL_OFFSET.x, settings.STOP_LABEL_OFFSET.y });
        result_back.SetFontSize(settings.STOP_LABEL_FONT_SIZE);
        result_back.SetFontFamily("Verdana");
        result_back.SetFontWeight("");
        result_back.SetData(text);

        result_text = result_back;

        result_back.SetFillColor(settings.UNDERLAYER_COLOR);
        result_back.SetStrokeColor(settings.UNDERLAYER_COLOR);
        result_back.SetStrokeWidth(settings.UNDERLAYER_WIDTH);
        result_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        result_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        result_text.SetFillColor("black");

        return { result_back, result_text };
    }


    void BusesRenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings, const SphereProjector& zoom)
    {
        int color_line_index = 0;

        auto all_buses(catalog.GetAllBuses());
        std::sort(all_buses.begin(), all_buses.end(), [](Transport::Data::Bus* left, Transport::Data::Bus* right)
            { return left->name < right->name; });

        std::vector<svg::Text> buses_labels;
        buses_labels.reserve(all_buses.size() * 2);

        for (const auto bus : all_buses)
        {
            if (bus->stops.empty())
            {
                continue;
            }

            svg::Polyline route;

            std::vector<svg::Point> point_coords;
            point_coords.reserve(bus->stops.size() * (bus->is_roundtrip ? 1 : 2));
            for (const auto& stop : bus->stops)
            {
                svg::Point point = zoom(stop->coordinates);
                point_coords.push_back(point);
                route.AddPoint(point);
            }
            if (!bus->is_roundtrip && point_coords.size() >= 2)
            {
                for (int i = point_coords.size() - 2; i >= 0; --i)
                {
                    route.AddPoint(point_coords[i]);
                }
            }

            auto color = settings.COLOR_PALETTE[color_line_index % settings.COLOR_PALETTE.size()];
            route.SetStrokeColor(color);
            route.SetStrokeWidth(settings.LINE_WIDTH);
            route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            route.SetFillColor(svg::NoneColor);

            {
                auto [label_back, label] = MakeBusLabel(settings, color, point_coords[0], bus->name);
                buses_labels.push_back(std::move(label_back));
                buses_labels.push_back(std::move(label));
            }

            if (!bus->is_roundtrip && bus->stops[0] != bus->stops[bus->stops.size()-1])
            {
                auto [label_back, label] = MakeBusLabel(settings, color, point_coords[point_coords.size() - 1], bus->name);
                buses_labels.push_back(std::move(label_back));
                buses_labels.push_back(std::move(label));
            }

            document.Add(route);
            ++color_line_index;
        }

        for (auto& svg_text : buses_labels)
        {
            document.Add(std::move(svg_text));
        }
    }

    void StopsRenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings, const SphereProjector& zoom)
    {
        std::vector<Transport::Data::Stop*> all_stops(catalog.GetAllStops());
        std::sort(all_stops.begin(), all_stops.end(), [](Transport::Data::Stop* left, Transport::Data::Stop* right)
            { return left->name < right->name; });

        std::vector<svg::Text> stops_labels;
        stops_labels.reserve(all_stops.size());
      
        for (const auto stop : all_stops)
        {
            if (stop->buses.empty())
            {
                continue;
            }

            svg::Circle stop_place;

            svg::Point point = zoom(stop->coordinates);
            stop_place.SetCenter(point);
            stop_place.SetRadius(settings.STOP_RADIUS);
            stop_place.SetFillColor("white");            
           
            document.Add(stop_place);

            auto [label_back, label] = MakeStopLabel(settings, point, stop->name);
            stops_labels.push_back(std::move(label_back));
            stops_labels.push_back(std::move(label));
        }

        for (auto& svg_text : stops_labels)
        {
            document.Add(std::move(svg_text));
        }
    }

    void RenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings)
    {
        const SphereProjector zoom = MakeSphereProjector(catalog, settings);
        BusesRenderToDoc(document, catalog, settings, zoom);
        StopsRenderToDoc(document, catalog, settings, zoom);
    }
} //map_render
