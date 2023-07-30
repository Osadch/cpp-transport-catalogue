#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport {

	class Router {
	public:
		Router() = default;

		Router(const int bus_wait_time, const double bus_velocity)
			: bus_wait_time_(bus_wait_time)
			, bus_velocity_(bus_velocity) {}

		Router(const Router& settings, const TransportCatalogue& catalogue) {
			bus_wait_time_ = settings.bus_wait_time_;
			bus_velocity_ = settings.bus_velocity_;
			BuildGraph(catalogue); 
		}
		// Метод создания графа
		const graph::DirectedWeightedGraph<double>& BuildGraph(const TransportCatalogue& catalogue);
		// Метод поиска маршрута
		const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
		// Метод получения графа
		const graph::DirectedWeightedGraph<double>& GetGraph() const;

	private:
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0.0;

		graph::DirectedWeightedGraph<double> graph_;
		// map идентификаторов остановок
		std::map<std::string, graph::VertexId> stop_ids_;
		// Указатель на объект класса Router
		std::unique_ptr<graph::Router<double>> router_;
	};

}
