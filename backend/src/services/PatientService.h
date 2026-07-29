#pragma once

#include "repositories/PatientRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::services
{
    enum class PatientServiceErrorType
    {
        Validation,
        Conflict,
        Database
    };

    struct PatientServiceError
    {
        PatientServiceErrorType type;
        std::string field;
        std::string message;
    };

    class PatientService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        using CreateErrorCallback =
            std::function<void(PatientServiceError)>;

        explicit PatientService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listPatients(
            std::string searchTerm,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void createPatient(
            healthcare::models::CreatePatientInput input,
            SuccessCallback successCallback,
            CreateErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::PatientRepository
            patientRepository_;
    };
}