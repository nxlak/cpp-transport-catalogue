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
    
std::string MapRenderer::RenderSvg(const RenderSettings& settings, const TransportCatalogue& catalogue) {
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

    std::deque<Bus> buses = catalogue.GetBuses();
    std::sort(buses.begin(), buses.end(), [](const Bus& lhs, const Bus& rhs) {
        return lhs.name < rhs.name; 
    });
    
    //отрисовка линий маршрутов 
    size_t color_count = settings.color_palette.size();
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto& bus = buses[i];
        if (bus.stops.empty()) {
            continue;
        }

        svg::Polyline line;
        json::Node color_node = settings.color_palette[i % color_count];
        svg::Color color;

        if (color_node.IsArray()) {
            if (color_node.AsArray().size() == 3) {
                color = svg::Rgb(
                    static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(color_node.AsArray().at(2).AsInt())
                );
            } else if (color_node.AsArray().size() == 4) {
                color = svg::Rgba(
                    static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(color_node.AsArray().at(2).AsInt()),
                    color_node.AsArray().at(3).AsDouble()
                );
            }
        } else if (color_node.IsString()) {
            color = color_node.AsString();
        }

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
    
    // отрисовка названий маршрутов
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto& bus = buses[i];
        if (bus.stops.empty()) {
            continue;
        }

        std::vector<const Stop*> end_stops;
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

            json::Node underlayer_color_node = settings.underlayer_color;
            svg::Color underlayer_color;
            
            if (underlayer_color_node.IsArray()) {
                if (underlayer_color_node.AsArray().size() == 3) {
                    underlayer_color = svg::Rgb(
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt())
                    );
                } else if (underlayer_color_node.AsArray().size() == 4) {
                    underlayer_color = svg::Rgba(
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt()),
                        underlayer_color_node.AsArray().at(3).AsDouble()
                    );
                }
            } else if (underlayer_color_node.IsString()) {
                underlayer_color = underlayer_color_node.AsString();
            }

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
                .SetData(bus.name);

            json::Node color_node = settings.color_palette[i % settings.color_palette.size()];
            svg::Color color;

            if (color_node.IsArray()) {
                if (color_node.AsArray().size() == 3) {
                    color = svg::Rgb(
                        static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(2).AsInt())
                    );
                } else if (color_node.AsArray().size() == 4) {
                    color = svg::Rgba(
                        static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(2).AsInt()),
                        color_node.AsArray().at(3).AsDouble()
                    );
                }
            } else if (color_node.IsString()) {
                color = color_node.AsString();
            }

            text.SetFillColor(color);

            svg_doc.Add(underlayer_text);
            svg_doc.Add(text);
        }
    }
    
    // отрисовка символов остановок
    std::deque<Stop> all_stops = catalogue.GetStops();
    std::sort(all_stops.begin(), all_stops.end(), [](const Stop& lhs, const Stop& rhs) {
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
    
    // отрисовка названий остановок
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

        json::Node underlayer_color_node = settings.underlayer_color;
        svg::Color underlayer_color;

        if (underlayer_color_node.IsArray()) {
            if (underlayer_color_node.AsArray().size() == 3) {
                underlayer_color = svg::Rgb(
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt())
                );
            } else if (underlayer_color_node.AsArray().size() == 4) {
                underlayer_color = svg::Rgba(
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(underlayer_color_node.AsArray().at(2).AsInt()),
                    underlayer_color_node.AsArray().at(3).AsDouble()
                );
            }
        } else if (underlayer_color_node.IsString()) {
            underlayer_color = underlayer_color_node.AsString();
        }

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