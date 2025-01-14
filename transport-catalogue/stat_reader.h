#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace Transport
{
    namespace PrintCataloge
    {
        void ParseAndPrintStat(const Transport::Data::TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);
    }
}
