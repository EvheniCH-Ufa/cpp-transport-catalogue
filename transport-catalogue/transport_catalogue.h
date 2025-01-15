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
			double route_length;
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

		private:
			std::deque<Bus> buses_;
			std::deque<Stop> stops_;

			std::unordered_map <std::string_view, Bus*> buses_for_find_;
			std::unordered_map <std::string_view, Stop*> stops_for_find_;
		};
	}
}
