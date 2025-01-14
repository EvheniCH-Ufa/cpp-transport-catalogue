#include<unordered_set>

#include "transport_catalogue.h"

namespace Transport
{
	namespace Data
	{
		void TransportCatalogue::AddBus(const std::string& bus_name, std::vector<std::string_view>& stops_names)
		{
			std::string name;
			std::vector<Stop*> current_bus_stops;

			Coordinates last_stop_coordinate;
			bool first_stop = true;
			double route_len = 0.0;

			std::unordered_set<Stop*> uniq_stops;

			for (auto stop_name : stops_names) // probeg po ostanovkam
			{   // find stop
				auto it_find = std::find_if(stops_for_find_.begin(), stops_for_find_.end(),
					[stop_name](std::pair< std::string* const, Stop* >& val)
					{
						return stop_name == *val.first;
					});

				if (it_find == stops_for_find_.end())
				{
					continue;
				}
				current_bus_stops.push_back(it_find->second); // add in bus_stops current stop

				if (!first_stop) // calculate route len
				{
					route_len += ComputeDistance(last_stop_coordinate, it_find->second->coordinates);
				}
				first_stop = false;
				last_stop_coordinate = it_find->second->coordinates;

				uniq_stops.insert(it_find->second);
			}

			// create and insert new bus
			Bus bus = { bus_name, std::move(current_bus_stops), route_len, uniq_stops.size() };
			buses_.push_back(std::move(bus));

			// ptr for new bus in dec 
			auto last_bus = &(buses_.back());
			buses_for_find_.insert({ &((buses_.back()).name), last_bus });

			// for all stops this bus add ptr new bus 
			for (auto stop : uniq_stops)
			{
				(*stop).buses.push_back(last_bus);
			}
		}

		void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coordinates) //name + shirota + dolgota
		{
			Stop stop = { stop_name, coordinates };
			stops_.push_back(std::move(stop));

			stops_for_find_.insert({ &(stops_.back().name), &(stops_.back()) });
		}

		const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const
		{
			auto find_it = std::find_if(buses_for_find_.begin(), buses_for_find_.end(),
				[bus_name](const std::pair< std::string* const, Bus* >& val)
				{
					return bus_name == *val.first;
				});
			if (find_it != buses_for_find_.end())
			{
				return find_it->second;
			}
			return nullptr;
		}

		const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const
		{
			auto it_find = std::find_if(stops_for_find_.begin(), stops_for_find_.end(),
				[stop_name](const std::pair< std::string* const, Stop* >& val)
				{
					return stop_name == *val.first;
				});

			if (it_find != stops_for_find_.end())
			{
				return it_find->second;
			}
			return nullptr;
		}

		const std::vector<Bus*>& Stop::GetBuses() const
		{
			return buses;
		}
	}
}
