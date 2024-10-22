#include "json_reader.h"
#include <iostream>

int main() {
    try {
        transport_catalogue::TransportCatalogue catalogue;
        json_reader::JsonReader reader;
        reader.ReadJson(std::cin, catalogue, std::cout);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}