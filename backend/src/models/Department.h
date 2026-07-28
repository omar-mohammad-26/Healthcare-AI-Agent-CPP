#pragma once

#include <optional>
#include <string>

namespace healthcare::models
{
    struct Department
    {
        int id{};
        std::string name;
        std::optional<std::string> description;
    };
}
