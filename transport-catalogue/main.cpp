#include "transport_catalogue.h"
//#include "input_reader.h"
//#include "stat_reader.h"
#include "json_reader.h"

int main() {
	TransportGuide::TransportCatalogue tc;
	JSON::LoadFromJson(tc, std::cin, std::cout);

	return 0;
}