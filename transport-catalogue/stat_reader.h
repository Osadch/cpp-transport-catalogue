#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "transport_catalogue.h"

#include <iostream>

namespace transport {

void ProcessRequests(TransportCatalogue& catalogue);
void PrintRoute(std::string& line, TransportCatalogue& catalogue);
void PrintStop(std::string& line, TransportCatalogue& catalogue);

} // namespace transport
