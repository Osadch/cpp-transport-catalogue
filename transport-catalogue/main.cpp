// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "stat_reader.h"
#include "input_reader.h"

using namespace transport;

int main() {
    TransportCatalogue catalogue;
    FillTransportCatalogue(catalogue);
    ProcessRequests(catalogue);
}
