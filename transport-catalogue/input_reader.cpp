#include "input_reader.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>
#include <utility>
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
namespace transport_catalogue {
namespace input {
namespace detail { 
    
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

} //namespace detail 
    
void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) {
    // Реализуйте метод самостоятельно
    if (commands_.empty()) {
        return;
    }
    std::vector<CommandDescription> buffer;
    for (const auto& com : commands_) {
        //сначала добавляем все остановки, только потом пробегаем по буфферу с маршрутами
        if (com.command == "Bus") {
            buffer.push_back(com);
            continue;
        }
        TransportCatalogue::Stop new_stop;
        new_stop.name = com.id;
        new_stop.coord = detail::ParseCoordinates(com.description);
        catalogue.AddStop(std::move(new_stop));
    }
    
    for (const auto& buf : buffer) {
        TransportCatalogue::Bus new_bus;
        new_bus.name = buf.id;
        std::vector<TransportCatalogue::Stop*> stops_for_bus;
        for (const std::string_view stop : detail::ParseRoute(buf.description)) {
            const TransportCatalogue::Stop* stop_ptr = catalogue.FindStop(stop);
            if (stop_ptr) {
                stops_for_bus.push_back(const_cast<TransportCatalogue::Stop*>(stop_ptr));
            }
        }
        new_bus.stops = stops_for_bus;
        catalogue.AddBus(std::move(new_bus));
    }
}
    
} //namespace input 
} //namespace transport_catalogue 
