#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#pragma once

#include <istream>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace Transport
{
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    class JsonReader {
    public:
        JsonReader() = delete;
        JsonReader(Transport::Data::TransportCatalogue& catalogue) : catalogue_(catalogue) {};

        void ReadJson(std::istream& input);

    private:
        std::vector<CommandDescription> commands_;
        Transport::Data::TransportCatalogue& catalogue_;

        void ApplyInsertJSONCommands(const json::Node& insert_array);
        json::Document ApplyStatJSONCommands(const json::Node& stat_array, const svg::Document& svg_document);

        map_render::RenderSettings ApplySettingsJSON(const json::Node& settings_array);
    };
} // namespace Transport
