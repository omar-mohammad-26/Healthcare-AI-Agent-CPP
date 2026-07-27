#pragma once

#include <string>

namespace healthcare::models
{
    struct Doctor
    {
        int id{};
        std::string fullName;
        std::string specialty;
        int departmentId{};
        std::string departmentName;
    };
}