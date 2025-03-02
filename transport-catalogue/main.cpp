// p10_01_15_TransportnyjSpravochnik_VizualizacijaKartyMarshrutov.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.

#include <iostream>
#include <string>
#include <fstream>


#include "json_reader.h"
//#include "transport_catalogue.h"


int main() {
    Transport::Data::TransportCatalogue catalogue;
    Transport::InputCataloge::Test(std::cin, catalogue);

}

   
