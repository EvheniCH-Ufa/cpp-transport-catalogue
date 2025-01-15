// p08_02_08_TransportnyjSpravochnik_PishemKod_3.cpp

#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    Transport::Data::TransportCatalogue catalogue;

    Transport::InputCataloge::Test(catalogue);
    Transport::PrintCataloge::Test(catalogue);
}
