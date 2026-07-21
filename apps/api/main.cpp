#include "apply-tracker/application/ApplicationService.hpp"
#include "apply-tracker/config/AppConfig.hpp"
#include "apply-tracker/domain/JobApplication.hpp"
#include "apply-tracker/infrastructure/GoogleAuth.hpp"
#include "apply-tracker/infrastructure/GoogleSheetsRepository.hpp"
#include "apply-tracker/infrastructure/HttpClient.hpp"

#include <drogon/drogon.h>

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

Json::Value applicationToJson(
    const apply_tracker::domain::JobApplication&
        application
) {
    Json::Value json;

    json["date"] = application.date;
    json["role"] = application.role;
    json["company"] = application.company;

    json["job_description"] =
        application.jobDescription;

    json["location"] = application.location;
    json["type"] = application.type;

    json["working_type"] =
        application.workingType;

    json["salary"] = application.salary;
    json["status"] = application.status;

    return json;
}

apply_tracker::domain::JobApplication
applicationFromJson(
    const Json::Value& json
) {
    apply_tracker::domain::JobApplication
        application;

    application.date =
        json.get("date", "").asString();

    application.role =
        json.get("role", "").asString();

    application.company =
        json.get("company", "").asString();

    application.jobDescription =
        json.get(
            "job_description",
            ""
        ).asString();

    application.location =
        json.get("location", "").asString();

    application.type =
        json.get("type", "").asString();

    application.workingType =
        json.get(
            "working_type",
            ""
        ).asString();

    application.salary =
        json.get("salary", "").asString();

    application.status =
        json.get(
            "status",
            "Applied"
        ).asString();

    return application;
}

drogon::HttpResponsePtr jsonError(
    drogon::HttpStatusCode status,
    const std::string& message
) {
    Json::Value body;
    body["success"] = false;
    body["error"] = message;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            body
        );

    response->setStatusCode(status);

    return response;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        const std::string configPath =
            argc > 1
                ? argv[1]
                : "config/app.json";

        const auto config =
            apply_tracker::config::AppConfig::load(
                configPath
            );

        auto httpClient =
            std::make_shared<
                apply_tracker::infrastructure::
                    HttpClient
            >();

        auto googleAuth =
            std::make_shared<
                apply_tracker::infrastructure::
                    GoogleAuth
            >(
                *httpClient,
                config.serviceAccountFile
            );

        auto repository =
            std::make_shared<
                apply_tracker::infrastructure::
                    GoogleSheetsRepository
            >(
                *httpClient,
                *googleAuth,
                config.spreadsheetId,
                config.worksheetName
            );

        auto service =
            std::make_shared<
                apply_tracker::application::
                    ApplicationService
            >(
                *repository
            );

        drogon::app().registerHandler(
            "/health",
            [](
                const drogon::HttpRequestPtr&,
                std::function<
                    void(
                        const drogon::HttpResponsePtr&
                    )
                >&& callback
            ) {
                Json::Value body;
                body["status"] = "ok";

                callback(
                    drogon::HttpResponse::
                        newHttpJsonResponse(body)
                );
            },
            {
                drogon::Get
            }
        );

        drogon::app().registerHandler(
            "/applications",
            [
                service,
                repository,
                googleAuth,
                httpClient
            ](
                const drogon::HttpRequestPtr&,
                std::function<
                    void(
                        const drogon::HttpResponsePtr&
                    )
                >&& callback
            ) {
                try {
                    const auto applications =
                        service->listApplications();

                    Json::Value responseBody;
                    responseBody["success"] = true;

                    responseBody["count"] =
                        static_cast<Json::UInt64>(
                            applications.size()
                        );

                    Json::Value items{
                        Json::arrayValue
                    };

                    for (
                        const auto& application :
                        applications
                    ) {
                        items.append(
                            applicationToJson(
                                application
                            )
                        );
                    }

                    responseBody["applications"] =
                        items;

                    callback(
                        drogon::HttpResponse::
                            newHttpJsonResponse(
                                responseBody
                            )
                    );
                } catch (
                    const std::exception& exception
                ) {
                    callback(
                        jsonError(
                            drogon::
                                k500InternalServerError,
                            exception.what()
                        )
                    );
                }
            },
            {
                drogon::Get
            }
        );

        drogon::app().registerHandler(
            "/applications",
            [
                service,
                repository,
                googleAuth,
                httpClient
            ](
                const drogon::HttpRequestPtr& request,
                std::function<
                    void(
                        const drogon::HttpResponsePtr&
                    )
                >&& callback
            ) {
                try {
                    const auto json =
                        request->getJsonObject();

                    if (!json) {
                        callback(
                            jsonError(
                                drogon::k400BadRequest,
                                "Request body must be valid JSON"
                            )
                        );

                        return;
                    }

                    const auto application =
                        applicationFromJson(*json);

                    service->addApplication(
                        application
                    );

                    Json::Value responseBody;
                    responseBody["success"] = true;

                    responseBody["message"] =
                        "Application added successfully";

                    auto response =
                        drogon::HttpResponse::
                            newHttpJsonResponse(
                                responseBody
                            );

                    response->setStatusCode(
                        drogon::k201Created
                    );

                    callback(response);
                } catch (
                    const std::invalid_argument&
                        exception
                ) {
                    callback(
                        jsonError(
                            drogon::k400BadRequest,
                            exception.what()
                        )
                    );
                } catch (
                    const std::exception& exception
                ) {
                    callback(
                        jsonError(
                            drogon::
                                k500InternalServerError,
                            exception.what()
                        )
                    );
                }
            },
            {
                drogon::Post
            }
        );

        drogon::app()
            .addListener(
                "0.0.0.0",
                config.apiPort
            )
            .setThreadNum(4)
            .run();

        return 0;
    } catch (const std::exception& exception) {
        std::cerr
            << "Fatal error: "
            << exception.what()
            << '\n';

        return 1;
    }
}