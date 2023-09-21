#pragma once
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "domain.h"
#include <filesystem>

void Serialize(const TransportGuide::TransportCatalogue& tc, const std::filesystem::path& path);
void Deserialize(TransportGuide::TransportCatalogue& tc, const std::filesystem::path& path);