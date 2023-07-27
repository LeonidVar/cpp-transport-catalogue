#include "transport_catalogue.h"
//#include "input_reader.h"
//#include "stat_reader.h"
#include "json_reader.h"

int main() {
	//json::Builder{}.StartDict().Key("1").Value(1).Value(1);  // правило 2
	TransportGuide::TransportCatalogue tc;
	JSON::JsonReader jr(tc, std::cin, std::cout);
	jr.LoadFromJson();

	return 0;
}