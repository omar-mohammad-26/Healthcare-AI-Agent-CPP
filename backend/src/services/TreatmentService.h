#pragma once

#include "repositories/TreatmentRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <optional>
#include <string>

namespace healthcare::services
{
    class TreatmentService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit TreatmentService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listTreatments(
            std::optional<int> patientId,
            std::optional<int> doctorId,
            std::string searchTerm,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::TreatmentRepository
            treatmentRepository_;
    };
}