#pragma once

#include "models/Department.h"

#include <drogon/orm/DbClient.h>

#include <functional>
#include <string>
#include <vector>

namespace healthcare::repositories
{
    class DepartmentRepository final
    {
    public:
        using DepartmentList =
            std::vector<healthcare::models::Department>;

        using SuccessCallback =
            std::function<void(DepartmentList)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit DepartmentRepository(
            drogon::orm::DbClientPtr databaseClient
        );

        void findAll(
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}