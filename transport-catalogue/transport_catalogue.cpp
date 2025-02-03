#include<unordered_set>

#include "transport_catalogue.h"

namespace Transport
{
	namespace Data
	{
		void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_names)
		{
			std::string name;
			std::vector<Stop*> current_bus_stops;

			Coordinates last_stop_coordinate;
			std::string last_stop;

			bool first_stop = true;
			RouteLength route_len;

			std::unordered_set<Stop*> uniq_stops;

			for (auto stop_name : stops_names) // probeg po ostanovkam
			{   // find stop

				auto it_find = stops_for_find_.find(stop_name);
				if (it_find == stops_for_find_.end())
				{
					continue;
				}
				current_bus_stops.push_back(it_find->second); // add in bus_stops current stop

				if (!first_stop) // calculate route len
				{
					route_len.geo += ComputeDistance(last_stop_coordinate, it_find->second->coordinates);
					route_len.street += GetLenBetweenStops(last_stop, std::string(stop_name));
				}
				first_stop = false;
				last_stop_coordinate = it_find->second->coordinates;
				last_stop = std::string(stop_name);

				uniq_stops.insert(it_find->second);
			}

			// create and insert new bus
			Bus bus = { bus_name, std::move(current_bus_stops), route_len, uniq_stops.size() };
			buses_.push_back(std::move(bus));

			// ptr for new bus in dec 
			auto last_bus = &(buses_.back());
			buses_for_find_.insert({ last_bus->name, last_bus });

			// for all stops this bus add ptr new bus 
			for (auto stop : uniq_stops)
			{
				(*stop).buses.insert(bus_name);
			}
		}

		void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coordinates) //name + shirota + dolgota
		{
			Stop stop = { stop_name, coordinates };
			stops_.push_back(std::move(stop));
			stops_for_find_.insert({ stops_.back().name, &(stops_.back()) }); // можжет тут быть
		}

		const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const
		{
			auto find_it = buses_for_find_.find(bus_name);
			if (find_it != buses_for_find_.end())
			{
				return find_it->second;
			}
			return nullptr;
		}

		const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const
		{
			auto it_find = stops_for_find_.find(stop_name);
			if (it_find != stops_for_find_.end())
			{
				return it_find->second;
			}
			return nullptr;
		}

		void TransportCatalogue::AddLenBetweenStops(std::string_view stop_from, std::string_view stop_to, int length)
		{
			std::string key = std::string(stop_from) + ___TO___ + std::string(stop_to);
			lengths_between_stops_[key] = length;
		};

		int TransportCatalogue::GetLenBetweenStops(const std::string& stop_from, const std::string& stop_to) const
		{
			std::string key = stop_from + ___TO___ + stop_to;
			auto it_find = lengths_between_stops_.find(key);

			if (it_find != lengths_between_stops_.end())
			{
				return it_find->second;
			}

			key = stop_to + ___TO___ + stop_from;
			it_find = lengths_between_stops_.find(key);

			if (it_find != lengths_between_stops_.end())
			{
				return it_find->second;
			}

			return 0;
		};

		const std::set<std::string>& Stop::GetBuses() const
		{
			return buses;
		}
	}
}
