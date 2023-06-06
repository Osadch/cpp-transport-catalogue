#include "stat_reader.h"
#include "input_reader.h"

using namespace transport;

int main() {
    Catalogue catalogue;
    FillCatalogue(std::cin, catalogue);
    ProcessRequests(std::cin, catalogue, std::cout);
}
