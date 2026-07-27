#pragma once

#include "repositories/DepartmentRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::services
{
    class DepartmentService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit DepartmentService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listDepartments(
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::DepartmentRepository
            departmentRepository_;
    };
}