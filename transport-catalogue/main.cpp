// p12_04_07_TransportSpravoch_PoderzkaMarsh_v.cpp

#include <iostream>
#include <string>
#include <fstream>


#include "json_reader.h"

int main() {
    Transport::Data::TransportCatalogue catalogue;

    //auto in_file = std::ifstream("З_1.json"); // pri razrabotke

    Transport::JsonReader json_reader(catalogue); // ++++
   // json_reader.ReadJson(in_file);                // pri razrabotke

    json_reader.ReadJson(std::cin);         // pri sdache  

   // std::cout << "end";                          // pri razrabotke

}
