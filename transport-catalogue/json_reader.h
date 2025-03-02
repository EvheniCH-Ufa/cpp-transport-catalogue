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
    namespace InputCataloge
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

        class InputReader {
        public:
            /**
             * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
             */
            void ParseLine(std::string_view line);

            /**
             * Наполняет данными транспортный справочник, используя команды из commands_
             */
            void ApplyCommands(Transport::Data::TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };

        void ApplyInsertJSONCommands([[maybe_unused]] Transport::Data::TransportCatalogue& catalogue, const json::Node& insert_array);
        json::Document ApplyStatJSONCommands(Transport::Data::TransportCatalogue& catalogue, const json::Node& stat_array, const svg::Document& svg_document);

        map_render::RenderSettings ApplySettingsJSON(const json::Node& settings_array);

        void Test(std::istream& input, Transport::Data::TransportCatalogue& catalogue);
    } // namespace InputCataloge
} // namespace Transport
