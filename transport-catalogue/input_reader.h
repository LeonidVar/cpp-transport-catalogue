#pragma once
#include <string>
#include <iostream>
#include "transport_catalogue.h"

namespace TransportGuide {
namespace input{
//Считывает входящий запрос и по первому слову Bus/Stop вызывает соответсвующую функцию добавления
void Request(TransportCatalogue& tc, std::string s);
//Добавить в базу остановку
void StopX(TransportCatalogue& tc, std::string s);
//Добавить в базу маршрут автобуса
void BusX(TransportCatalogue& tc, std::string s);

//Считывает строку с остановками с заданным разделителем, возвращает контейнер с именами остановок
template <typename Container>
void ReadStopsFromLine(std::string& s, char divider, Container& cont) {
	size_t pos{ 0 };
	if (s.find(divider) != std::string::npos) {
		while (true) {
			size_t pos0 = pos;
			pos = s.find(divider, pos);
			if (pos != std::string::npos) {
				cont.push_back(s.substr(pos0, pos - 1 - pos0));
				pos += 2;
			}
			else {
				cont.push_back(s.substr(pos0));
				break;
			}

		}
	}
}
}
}