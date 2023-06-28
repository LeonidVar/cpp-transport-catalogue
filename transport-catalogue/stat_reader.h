#pragma once
#include <iostream>
#include <iomanip>  
#include "transport_catalogue.h"

namespace TransportGuide {
namespace stat {

//Считывает входящий запрос и по первому слову Bus/Stop вызывает соответсвующую функцию вывода
void ReceiveRequest(const TransportCatalogue& tc, std::string s);
//Вывод информации о маршруте
void PrintRoute(const TransportCatalogue& tc, std::string bus);
//Вывод информации об остановке
void PrintStop(const TransportCatalogue& tc, std::string stop);

}
}