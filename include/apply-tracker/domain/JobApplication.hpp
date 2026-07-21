#pragma once

#include <string>

namespace apply_tracker::domain {

struct JobApplication {
    std::string date;
    std::string role;
    std::string company;
    std::string jobDescription;
    std::string location;
    std::string type;
    std::string workingType;
    std::string salary;
    std::string status{"Applied"};
};

} // namespace apply_tracker::domain