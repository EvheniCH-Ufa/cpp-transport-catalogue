#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 */
namespace map_render {

    const SphereProjector MapRenderer::MakeSphereProjector()
    {
        std::vector<geo::Coordinates> geo_coords;
        auto all_buses = catalog_.GetAllBuses();
        for (const auto& bus : all_buses)
        {
            for (const auto& stop : bus->stops)
            {
                geo_coords.push_back(stop->coordinates);
            }
        }
        return SphereProjector{ geo_coords.begin(), geo_coords.end(), settings_.WIDTH, settings_.HEIGHT, settings_.PADDING };
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


    void MapRenderer::BusesRenderToDoc(svg::Document& document)
    {
        int color_line_index = 0;

        auto all_buses(catalog_.GetAllBuses());
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

            const auto [first_point, last_point] = MakeBusRoute(route, *bus);
            const auto color = settings_.COLOR_PALETTE[color_line_index % settings_.COLOR_PALETTE.size()];
            SetBusRouteSettings(route, color);

            {
                auto [label_back, label] = MakeBusLabel(settings_, color, first_point, bus->name);
                buses_labels.push_back(std::move(label_back));
                buses_labels.push_back(std::move(label));
            }

            if (!bus->is_roundtrip && bus->stops[0] != bus->stops[bus->stops.size()-1])
            {
                auto [label_back, label] = MakeBusLabel(settings_, color, last_point, bus->name);
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

    void MapRenderer::SetBusRouteSettings(svg::Polyline& route, const svg::Color& color)
    {
        route.SetStrokeColor(color);
        route.SetStrokeWidth(settings_.LINE_WIDTH);
        route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        route.SetFillColor(svg::NoneColor);
    }

    const std::pair<svg::Point, svg::Point> MapRenderer::MakeBusRoute(svg::Polyline& route, const Transport::Data::Bus& bus)
    {
        std::vector<svg::Point> point_coords;

        point_coords.reserve(bus.stops.size() * (bus.is_roundtrip ? 1 : 2));
        for (const auto& stop : bus.stops)
        {
            svg::Point point = sphere_projector_(stop->coordinates);
            point_coords.push_back(point);
            route.AddPoint(point);
        }
        if (!bus.is_roundtrip && point_coords.size() >= 2)
        {
            for (int i = point_coords.size() - 2; i >= 0; --i)
            {
                route.AddPoint(point_coords[i]);
            }
        }
        return { point_coords[0], point_coords.back() };
    }


    void MapRenderer::SetStopProperty(svg::Circle& stop_place, svg::Point point)
    {
        stop_place.SetCenter(point);
        stop_place.SetRadius(settings_.STOP_RADIUS);
        stop_place.SetFillColor("white");
    }


    void MapRenderer::StopsRenderToDoc(svg::Document& document)
    {
        std::vector<Transport::Data::Stop*> all_stops(catalog_.GetAllStops());
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

            svg::Point stop_centre = sphere_projector_(stop->coordinates);
            svg::Circle stop_place;
            SetStopProperty(stop_place, stop_centre);

            document.Add(stop_place);

            auto [label_back, label] = MakeStopLabel(settings_, stop_centre, stop->name);
            stops_labels.push_back(std::move(label_back));
            stops_labels.push_back(std::move(label));
        }

        for (auto& svg_text : stops_labels)
        {
            document.Add(std::move(svg_text));
        }
    }

    void MapRenderer::RenderToSVGDoc(svg::Document& document)
    {
        BusesRenderToDoc(document);
        StopsRenderToDoc(document);
    }
} //map_render
