#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

#include "geo.h"



namespace Transport
{
	namespace Data
	{
		struct Bus;

		struct RouteLength
		{
			double geo = 0.0;
			int street = 0;
		};

		struct Stop
		{
			std::string name;
			Coordinates coordinates;
			const std::set<std::string>& GetBuses() const;
			std::set<std::string> buses = {};
		};

		struct Bus
		{
			std::string name;
			std::vector<Stop*> stops;
			RouteLength route_length;
			size_t unique_stop_count;
		};


		class TransportCatalogue
		{
			// Реализуйте класс самостоятельно
		public:
			void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_names);
			void AddStop(const std::string& stop_name, Coordinates coordinates); //name + shirota + dolgota
			const Bus* GetBus(std::string_view bus_name) const;
			const Stop* GetStop(std::string_view stop_name) const;
			void AddLenBetweenStops(std::string_view stop_from, std::string_view stop_to, int length);
			int GetLenBetweenStops(const std::string& stop_from, const std::string& stop_to) const;


		private:
			const std::string ___TO___ = "___to___";

			std::deque<Bus> buses_;
			std::deque<Stop> stops_;

			std::unordered_map <std::string_view, Bus*> buses_for_find_;
			std::unordered_map <std::string_view, Stop*> stops_for_find_;
			std::unordered_map<std::string, int> lengths_between_stops_;
		};
	}
}
