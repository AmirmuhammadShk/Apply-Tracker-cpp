#include "apply-tracker/application/ApplicationService.hpp"
#include "apply-tracker/domain/ApplicationRepository.hpp"
#include "apply-tracker/domain/JobApplication.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class FakeApplicationRepository final
    : public apply_tracker::domain::ApplicationRepository {
public:
    void add(
        const apply_tracker::domain::JobApplication& application
    ) override {
        applications.push_back(application);
    }

    std::vector<apply_tracker::domain::JobApplication>
    list() override {
        return applications;
    }

    std::vector<apply_tracker::domain::JobApplication>
        applications;
};

void expect(
    bool condition,
    const std::string& message
) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template<typename Function>
void expectInvalidArgument(
    Function function,
    const std::string& expectedMessage
) {
    try {
        function();
    } catch (const std::invalid_argument& exception) {
        expect(
            std::string{exception.what()} == expectedMessage,
            "Unexpected exception message: " +
                std::string{exception.what()}
        );

        return;
    }

    throw std::runtime_error(
        "Expected std::invalid_argument"
    );
}

apply_tracker::domain::JobApplication
validApplication() {
    apply_tracker::domain::JobApplication application;

    application.date = "2026-07-21";
    application.role = "C++ Software Engineer";
    application.company = "Example Company";

    application.jobDescription =
        "Develop backend services in modern C++";

    application.location = "Cambridge";
    application.type = "Permanent";
    application.workingType = "Hybrid";
    application.salary = "£70,000";
    application.status = "Applied";

    return application;
}

void testAddsValidApplication() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    const auto application = validApplication();

    service.addApplication(application);

    expect(
        repository.applications.size() == 1,
        "Application was not added"
    );

    expect(
        repository.applications[0].role ==
            "C++ Software Engineer",
        "Incorrect role was stored"
    );

    expect(
        repository.applications[0].company ==
            "Example Company",
        "Incorrect company was stored"
    );
}

void testAddsDefaultDate() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.date.clear();

    service.addApplication(application);

    expect(
        repository.applications.size() == 1,
        "Application was not added"
    );

    expect(
        !repository.applications[0].date.empty(),
        "Default date was not generated"
    );

    expect(
        repository.applications[0].date.size() == 10,
        "Generated date must use YYYY-MM-DD format"
    );
}

void testAddsDefaultStatus() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.status.clear();

    service.addApplication(application);

    expect(
        repository.applications[0].status == "Applied",
        "Default status was not applied"
    );
}

void testRejectsEmptyRole() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.role.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Role cannot be empty"
    );
}

void testRejectsEmptyCompany() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.company.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Company cannot be empty"
    );
}

void testRejectsEmptyJobDescription() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.jobDescription.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Job description cannot be empty"
    );
}

void testRejectsEmptyLocation() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.location.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Location cannot be empty"
    );
}

void testRejectsEmptyType() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.type.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Type cannot be empty"
    );
}

void testRejectsEmptyWorkingType() {
    FakeApplicationRepository repository;

    apply_tracker::application::ApplicationService service{
        repository
    };

    auto application = validApplication();
    application.workingType.clear();

    expectInvalidArgument(
        [&service, &application]() {
            service.addApplication(application);
        },
        "Working type cannot be empty"
    );
}

void testListsApplications() {
    FakeApplicationRepository repository;

    repository.applications.push_back(
        validApplication()
    );

    apply_tracker::application::ApplicationService service{
        repository
    };

    const auto applications =
        service.listApplications();

    expect(
        applications.size() == 1,
        "Incorrect application count"
    );

    expect(
        applications[0].company == "Example Company",
        "Incorrect application returned"
    );
}

} // namespace

int main() {
    try {
        testAddsValidApplication();
        testAddsDefaultDate();
        testAddsDefaultStatus();
        testRejectsEmptyRole();
        testRejectsEmptyCompany();
        testRejectsEmptyJobDescription();
        testRejectsEmptyLocation();
        testRejectsEmptyType();
        testRejectsEmptyWorkingType();
        testListsApplications();

        std::cout
            << "All ApplicationService tests passed.\n";

        return EXIT_SUCCESS;
    } catch (const std::exception& exception) {
        std::cerr
            << "Test failed: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }
}