#pragma once
#include <iostream>
#include <iomanip>  
#include "transport_catalogue.h"

namespace TransportGuide {
namespace stat {

//Считывает входящий запрос и по первому слову Bus/Stop вызывает соответсвующую функцию вывода
void ReceiveRequest(const TransportCatalogue& tc, const std::string& s, std::ostream& os = std::cout);
//Вывод информации о маршруте
void PrintRoute(const TransportCatalogue& tc, const std::string& bus, std::ostream& os);
//Вывод информации об остановке
void PrintStop(const TransportCatalogue& tc, const std::string& stop, std::ostream& os);

}
}