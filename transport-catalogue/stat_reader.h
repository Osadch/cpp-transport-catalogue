#pragma once
// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "transport_catalogue.h"

#include <iostream>

namespace transport {

void ProcessRequests(std::istream& in, Catalogue& catalogue, std::ostream& out);
void PrintRoute(std::string& line, Catalogue& catalogue, std::ostream& out);
void PrintStop(std::string& line, Catalogue& catalogue, std::ostream& out);

} // namespace transport
