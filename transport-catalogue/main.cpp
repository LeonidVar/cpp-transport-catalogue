#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
	TransportGuide::TransportCatalogue tc;
	int n;
	std::cin >> n;
	std::string s;
	

	while (n) {
		--n;
		std::getline(std::cin >> std::ws, s);
		TransportGuide::input::Request(tc, std::move(s));
	}
	tc.CompleteInput();

	std::cin >> n;
	while (n) {
		--n;
		std::getline(std::cin >> std::ws, s);
		TransportGuide::stat::ReceiveRequest(tc, std::move(s));
	}


//	std::cout << "OK\n";
	return 0;
}