#include "transport_catalogue.h"
#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

//int main() {
//	//json::Builder{}.StartDict().Key("1").Value(1).Value(1);  // правило 2
//	TransportGuide::TransportCatalogue tc;
//	JSON::JsonReader jr(tc, std::cin, std::cout);
//	jr.LoadFromJson();
//
//	return 0;
//}



using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);


    if (mode == "make_base"sv) {

        // make base here
        TransportGuide::TransportCatalogue tc;
        JSON::JsonReader jr(tc, std::cin, std::cout);
        jr.MakeBase();

    }
    else if (mode == "process_requests"sv) {

        // process requests here
        TransportGuide::TransportCatalogue tc;
        JSON::JsonReader jr(tc, std::cin, std::cout);
        jr.ProcessRequests();
    }
    else {
        PrintUsage();
        return 1;
    }
}