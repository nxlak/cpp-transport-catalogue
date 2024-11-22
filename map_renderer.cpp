#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

namespace map_renderer {
    
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
    
std::string MapRenderer::RenderSvg(const RenderSettings& settings, const transport_catalogue::TransportCatalogue& catalogue) {
    svg::Document svg_doc;

    std::vector<geo::Coordinates> route_stops;
    for (const auto& bus : catalogue.GetBuses()) {
        for (const auto& stop : bus.stops) {
            route_stops.push_back(stop->GetCoordinates());
        }
    }

    SphereProjector proj(route_stops.begin(), route_stops.end(), 
                          settings.width, settings.height, 
                          settings.padding);

    std::deque<transport_catalogue::Bus> buses = catalogue.GetBuses();
    std::sort(buses.begin(), buses.end(), [](const transport_catalogue::Bus& lhs, const transport_catalogue::Bus& rhs) {
        return lhs.name < rhs.name; 
    });

    size_t color_count = settings.color_palette.size();

    // Отрисовка линий маршрутов
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto& bus = buses[i];
        if (bus.stops.empty()) {
            continue;
        }

        svg::Polyline line;
        svg::Color color = settings.color_palette[i % color_count];
        line.SetStrokeColor(color)
           .SetStrokeWidth(settings.line_width)
           .SetFillColor(svg::NoneColor)
           .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
           .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto& stop : bus.stops) {
            line.AddPoint(proj(stop->GetCoordinates()));
        }
        
        svg_doc.Add(line);
    }

    // Отрисовка названий маршрутов
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto& bus = buses[i];
        if (bus.stops.empty()) {
            continue;
        }

        std::vector<const transport_catalogue::Stop*> end_stops;
        if (bus.is_roundtrip) {
            end_stops.push_back(bus.stops.front()); 
        } else if (!bus.is_roundtrip && bus.stops.front() != bus.last_elem) {
            end_stops.push_back(bus.stops.front());
            end_stops.push_back(bus.last_elem);
        }
        else {
            end_stops.push_back(bus.stops.front());
        }

        for (const auto& stop : end_stops) {
            svg::Text underlayer_text;
            underlayer_text.SetPosition(proj(stop->GetCoordinates()))
                            .SetOffset(svg::Point{settings.bus_label_offset.first, settings.bus_label_offset.second})
                            .SetFontSize(settings.bus_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetFontWeight("bold")
                            .SetData(bus.name);

            svg::Color underlayer_color = settings.underlayer_color;
            underlayer_text.SetFillColor(underlayer_color)
                            .SetStrokeColor(underlayer_color)
                            .SetStrokeWidth(settings.underlayer_width)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            svg::Text text;
            text.SetPosition(proj(stop->GetCoordinates()))
                .SetOffset(svg::Point{settings.bus_label_offset.first, settings.bus_label_offset.second})
                .SetFontSize(settings.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus.name)
                .SetFillColor(settings.color_palette[i % color_count]);

            svg_doc.Add(underlayer_text);
            svg_doc.Add(text);
        }
    }

    // Отрисовка символов остановок
    std::deque<transport_catalogue::Stop> all_stops = catalogue.GetStops();
    std::sort(all_stops.begin(), all_stops.end(), [](const transport_catalogue::Stop& lhs, const transport_catalogue::Stop& rhs) {
        return lhs.name < rhs.name;
    });

    for (const auto& stop : all_stops) {
        if (catalogue.GetBusesByStop(stop.name).empty()) {
            continue; 
        }

        svg::Circle circle;
        circle.SetCenter(proj(stop.GetCoordinates()))
              .SetRadius(settings.stop_radius)
              .SetFillColor("white");

        svg_doc.Add(circle);
    }

    // Отрисовка названий остановок
    for (const auto& stop : all_stops) {
        if (catalogue.GetBusesByStop(stop.name).empty()) {
            continue; 
        }

        svg::Text underlayer_text;
        underlayer_text.SetPosition(proj(stop.GetCoordinates()))
                      .SetOffset(svg::Point{settings.stop_label_offset.first, settings.stop_label_offset.second})
                      .SetFontSize(settings.stop_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetData(stop.name);

        svg::Color underlayer_color = settings.underlayer_color;
        underlayer_text.SetFillColor(underlayer_color)
                      .SetStrokeColor(underlayer_color)
                      .SetStrokeWidth(settings.underlayer_width)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text text;
        text.SetPosition(proj(stop.GetCoordinates()))
            .SetOffset(svg::Point{settings.stop_label_offset.first, settings.stop_label_offset.second})
            .SetFontSize(settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop.name)
            .SetFillColor("black");

        svg_doc.Add(underlayer_text);
        svg_doc.Add(text);
    }

    std::ostringstream svg_stream;
    svg_doc.Render(svg_stream);
    return svg_stream.str();
}
    
} // namespace map_renderer