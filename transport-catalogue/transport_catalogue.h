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
			geo::Coordinates coordinates;
			const std::set<std::string>& GetBuses() const;
			std::set<std::string> buses = {};
		};

		struct Bus
		{
			std::string name;
			std::vector<Stop*> stops;
			bool is_roundtrip = false;

			RouteLength route_length;
			size_t unique_stop_count;
		};


		class TransportCatalogue
		{
			// Реализуйте класс самостоятельно
		public:
			void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_names, bool is_roundtrip);
			void AddStop(const std::string& stop_name, geo::Coordinates coordinates); //name + shirota + dolgota
			const Bus* GetBus(std::string_view bus_name) const;
			Stop* GetStop(std::string_view stop_name) const;
			void AddDistBetweenStops(std::string_view stop_from, std::string_view stop_to, int length);
			int GetDistBetweenStops(const std::string& stop_from, const std::string& stop_to) const;

			const std::vector<Bus*>& GetAllBuses() const;
			const std::vector<Stop*>& GetAllStops() const;


		private:
			std::deque<Bus> buses_;
			std::deque<Stop> stops_;

			std::unordered_map <std::string_view, Bus*> buses_for_find_;
			std::unordered_map <std::string_view, Stop*> stops_for_find_;

			std::vector<Bus*>  buses_for_sort_;
			std::vector<Stop*> stops_for_sort_;

			struct PairStringHash {
				size_t  operator() (const std::pair<std::string, std::string>& pr) const {
					return d_hasher_str(pr.first) + d_hasher_str(pr.first) * 37;
				}
			private:
				std::hash<std::string> d_hasher_str;
			};

			std::unordered_map<std::pair<std::string, std::string>, int, PairStringHash> distance_between_stops_;
		};
	}
}