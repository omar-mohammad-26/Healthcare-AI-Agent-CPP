#pragma once

#include "repositories/PatientRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::services
{
    class PatientService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit PatientService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listPatients(
            std::string searchTerm,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::PatientRepository
            patientRepository_;
    };
}