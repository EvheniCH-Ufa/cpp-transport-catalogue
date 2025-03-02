#pragma once

#include <vector>

#include "svg.h"
#include "transport_catalogue.h"

namespace map_render {

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    struct RenderSettings  // временно - распарсить из JSON
    {
        double WIDTH = 600.0;
        double HEIGHT = 400.0;
        double PADDING = 50.0;
        double STOP_RADIUS = 5;
        double LINE_WIDTH = 14;
        double UNDERLAYER_WIDTH = 3;
        int BUS_LABEL_FONT_SIZE = 20;
        int STOP_LABEL_FONT_SIZE = 20;
        svg::Point BUS_LABEL_OFFSET = { 7, 15 };
        svg::Point STOP_LABEL_OFFSET = { 7, -3 };
        svg::Color UNDERLAYER_COLOR = svg::Rgba{ 255, 255, 255, 0.85 };
        std::vector<svg::Color> COLOR_PALETTE;// = { "green", svg::Rgb(255, 160, 0), "red" };
    };

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    void BusesRenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings, const SphereProjector& zoom);
    void StopsRenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings, const SphereProjector& zoom);

    void RenderToDoc(svg::Document& document, const Transport::Data::TransportCatalogue& catalog, const RenderSettings& settings);
} //map_render
