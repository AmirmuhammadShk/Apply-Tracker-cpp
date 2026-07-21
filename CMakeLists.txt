cmake_minimum_required(VERSION 3.20)

project(
    apply_tracker_cpp
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    APPLY_TRACKER_BUILD_API
    "Build the Drogon HTTP API"
    ON
)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(
    apply_tracker_core
    src/application/ApplicationService.cpp
    src/config/AppConfig.cpp
    src/infrastructure/HttpClient.cpp
    src/infrastructure/GoogleAuth.cpp
    src/infrastructure/GoogleSheetsRepository.cpp
)

target_include_directories(
    apply_tracker_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    apply_tracker_core
    PUBLIC
    CURL::libcurl
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

target_compile_options(
    apply_tracker_core
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(
    apply-tracker-cli
    apps/cli/main.cpp
)

target_link_libraries(
    apply-tracker-cli
    PRIVATE
    apply_tracker_core
)

target_compile_options(
    apply-tracker-cli
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(APPLY_TRACKER_BUILD_API)
    find_package(Drogon CONFIG REQUIRED)

    add_executable(
        apply-tracker-api
        apps/api/main.cpp
    )

    target_link_libraries(
        apply-tracker-api
        PRIVATE
        apply_tracker_core
        Drogon::Drogon
    )

    target_compile_options(
        apply-tracker-api
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
if(APPLY_TRACKER_BUILD_TESTS)
    enable_testing()

    add_executable(
        application-service-tests
        tests/ApplicationServiceTests.cpp
    )

    target_link_libraries(
        application-service-tests
        PRIVATE
        apply_tracker_core
    )

    target_compile_options(
        application-service-tests
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME ApplicationServiceTests
        COMMAND application-service-tests
    )

    add_executable(
        app-config-tests
        tests/AppConfigTests.cpp
    )

    target_link_libraries(
        app-config-tests
        PRIVATE
        apply_tracker_core
    )

    target_compile_options(
        app-config-tests
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME AppConfigTests
        COMMAND app-config-tests
    )
endif()