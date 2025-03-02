#include<unordered_set>
#include<algorithm>

#include "transport_catalogue.h"

namespace Transport
{
	namespace Data
	{
		void TransportCatalogue::AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_names, bool is_roundtrip)
		{
			std::string name;
			std::vector<Stop*> current_bus_stops;

			geo::Coordinates last_stop_coordinate;
			std::string last_stop;

			bool first_stop = true;
			RouteLength route_len;

			std::unordered_set<Stop*> uniq_stops;

			/* probeg po ostanovkam: if round -> sam zamknetsa 
			*  иначе количество остановок = уникальное*2-1
			*  иначе длина маршрутов * 2 
			*/ 
			for (auto stop_name : stops_names) 
			{   // find stop

				auto it_find = stops_for_find_.find(stop_name);
				if (it_find == stops_for_find_.end())
				{
//					continue;
					return;
				}
				current_bus_stops.push_back(it_find->second); // add in bus_stops current stop

				if (!first_stop) // calculate route len
				{   //                если не круговой, то расстояние ГЕО удваиваем 
					route_len.geo += (is_roundtrip ? 1.0 : 2.0) * ComputeDistance(last_stop_coordinate, it_find->second->coordinates);

					route_len.street += GetDistBetweenStops(last_stop, std::string(stop_name));
					if (!is_roundtrip) // если не круговой, то ищем расстояние по улицам в обратку
					{
						route_len.street += GetDistBetweenStops(std::string(stop_name), last_stop);
					}
				}
				first_stop = false;
				last_stop_coordinate = it_find->second->coordinates;
				last_stop = std::string(stop_name);

				uniq_stops.insert(it_find->second);
			}

			// create and insert new bus
			Bus bus = { bus_name, std::move(current_bus_stops), is_roundtrip, route_len, uniq_stops.size() };
			buses_.push_back(std::move(bus));

			// ptr for new bus in dec 
			auto last_bus = &(buses_.back());
			buses_for_find_.insert({ last_bus->name, last_bus });
			buses_for_sort_.push_back(last_bus);

			// for all stops this bus add ptr new bus 
			for (auto stop : uniq_stops)
			{
				(*stop).buses.insert(bus_name);
			}
		}

		void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates coordinates) //name + shirota + dolgota
		{
			Stop stop = { stop_name, coordinates };
			stops_.push_back(std::move(stop));
			stops_for_find_.insert({ stops_.back().name, &(stops_.back()) }); // можжет тут быть
			stops_for_sort_.push_back(&(stops_.back()));
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

		void TransportCatalogue::AddDistBetweenStops(std::string_view stop_from, std::string_view stop_to, int length)
		{
			distance_between_stops_[{std::string(stop_from), std::string(stop_to)}] = length;
		};

		int TransportCatalogue::GetDistBetweenStops(const std::string& stop_from, const std::string& stop_to) const
		{
			auto it_find = distance_between_stops_.find({ stop_from, stop_to });

			if (it_find != distance_between_stops_.end())
			{
				return it_find->second;
			}

			it_find = distance_between_stops_.find({ stop_to, stop_from });

			if (it_find != distance_between_stops_.end())
			{
				return it_find->second;
			}

			return 0;
		}
		const std::vector<Bus*>& TransportCatalogue::GetAllBuses() const
		{
			return buses_for_sort_;
		}

		const std::vector<Stop*>& TransportCatalogue::GetAllStops() const
		{
			return stops_for_sort_;// TODO: вставьте здесь оператор return
		}

		const std::set<std::string>& Stop::GetBuses() const
		{
			return buses;
		}
	}
}
