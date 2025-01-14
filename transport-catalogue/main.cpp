// p08_02_08_TransportnyjSpravochnik_PishemKod_3.cpp

#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    Transport::Data::TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        Transport::InputCataloge::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        Transport::PrintCataloge::ParseAndPrintStat(catalogue, line, cout);
    }
}
