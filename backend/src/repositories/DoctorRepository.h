#pragma once

#include "models/Doctor.h"

#include <drogon/orm/DbClient.h>

#include <functional>
#include <string>
#include <vector>

namespace healthcare::repositories
{
    class DoctorRepository final
    {
    public:
        using DoctorList =
            std::vector<healthcare::models::Doctor>;

        using SuccessCallback =
            std::function<void(DoctorList)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit DoctorRepository(
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