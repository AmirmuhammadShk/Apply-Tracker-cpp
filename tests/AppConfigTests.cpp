#include "apply-tracker/config/AppConfig.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void expect(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void writeFile(
    const std::filesystem::path& path,
    const std::string& content
) {
    std::ofstream output{path};

    if (!output) {
        throw std::runtime_error(
            "Failed to create test configuration"
        );
    }

    output << content;
}

void testLoadsValidConfig() {
    const std::filesystem::path path =
        "test-app-config.json";

    writeFile(
        path,
        R"({
            "spreadsheet_id": "spreadsheet-123",
            "worksheet_name": "Applies",
            "service_account_file": "config/service_account.json",
            "api_port": 9090
        })"
    );

    const auto config =
        apply_tracker::config::AppConfig::load(path);

    std::filesystem::remove(path);

    expect(
        config.spreadsheetId == "spreadsheet-123",
        "Incorrect spreadsheet ID"
    );

    expect(
        config.worksheetName == "Applies",
        "Incorrect worksheet name"
    );

    expect(
        config.serviceAccountFile ==
            std::filesystem::path{
                "config/service_account.json"
            },
        "Incorrect service-account path"
    );

    expect(
        config.apiPort == 9090,
        "Incorrect API port"
    );
}

void testUsesDefaultValues() {
    const std::filesystem::path path =
        "test-default-config.json";

    writeFile(
        path,
        R"({
            "spreadsheet_id": "spreadsheet-123",
            "service_account_file": "service-account.json"
        })"
    );

    const auto config =
        apply_tracker::config::AppConfig::load(path);

    std::filesystem::remove(path);

    expect(
        config.worksheetName == "Sheet1",
        "Default worksheet name was not used"
    );

    expect(
        config.apiPort == 8080,
        "Default API port was not used"
    );
}

void testRejectsMissingFile() {
    try {
        apply_tracker::config::AppConfig::load(
            "file-that-does-not-exist.json"
        );
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error(
        "Expected missing configuration file error"
    );
}

void testRejectsEmptySpreadsheetId() {
    const std::filesystem::path path =
        "test-empty-spreadsheet.json";

    writeFile(
        path,
        R"({
            "spreadsheet_id": "",
            "worksheet_name": "Applies",
            "service_account_file": "service-account.json"
        })"
    );

    try {
        apply_tracker::config::AppConfig::load(path);
    } catch (const std::runtime_error& exception) {
        std::filesystem::remove(path);

        expect(
            std::string{exception.what()} ==
                "spreadsheet_id cannot be empty",
            "Unexpected exception message"
        );

        return;
    }

    std::filesystem::remove(path);

    throw std::runtime_error(
        "Expected empty spreadsheet ID error"
    );
}

void testRejectsEmptyWorksheetName() {
    const std::filesystem::path path =
        "test-empty-worksheet.json";

    writeFile(
        path,
        R"({
            "spreadsheet_id": "spreadsheet-123",
            "worksheet_name": "",
            "service_account_file": "service-account.json"
        })"
    );

    try {
        apply_tracker::config::AppConfig::load(path);
    } catch (const std::runtime_error& exception) {
        std::filesystem::remove(path);

        expect(
            std::string{exception.what()} ==
                "worksheet_name cannot be empty",
            "Unexpected exception message"
        );

        return;
    }

    std::filesystem::remove(path);

    throw std::runtime_error(
        "Expected empty worksheet name error"
    );
}

} // namespace

int main() {
    try {
        testLoadsValidConfig();
        testUsesDefaultValues();
        testRejectsMissingFile();
        testRejectsEmptySpreadsheetId();
        testRejectsEmptyWorksheetName();

        std::cout
            << "All AppConfig tests passed.\n";

        return EXIT_SUCCESS;
    } catch (const std::exception& exception) {
        std::cerr
            << "Test failed: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }
}