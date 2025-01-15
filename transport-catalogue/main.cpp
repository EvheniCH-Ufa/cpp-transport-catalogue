// p08_02_08_TransportnyjSpravochnik_PishemKod_3.cpp

#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    Transport::Data::TransportCatalogue catalogue;

    Transport::InputCataloge::Test(std::cin, catalogue);
    Transport::PrintCataloge::Test(std::cin, std::cout, catalogue);
}
