#pragma once

#include "models/Patient.h"

#include <drogon/orm/DbClient.h>

#include <functional>
#include <string>
#include <vector>

namespace healthcare::repositories
{
    class PatientRepository final
    {
    public:
        using PatientList =
            std::vector<healthcare::models::Patient>;

        using SuccessCallback =
            std::function<void(PatientList)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit PatientRepository(
            drogon::orm::DbClientPtr databaseClient
        );

        void findAll(
            std::string searchTerm,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}