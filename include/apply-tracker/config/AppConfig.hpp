#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace apply_tracker::config {

struct AppConfig {
    std::string spreadsheetId;
    std::string worksheetName;
    std::filesystem::path serviceAccountFile;
    std::uint16_t apiPort{8080};

    static AppConfig load(
        const std::filesystem::path& path
    );
};

} // namespace apply_tracker::config