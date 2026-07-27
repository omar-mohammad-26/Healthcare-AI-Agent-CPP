#pragma once

#include "models/Treatment.h"

#include <drogon/orm/DbClient.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace healthcare::repositories
{
    class TreatmentRepository final
    {
    public:
        using TreatmentList =
            std::vector<healthcare::models::Treatment>;

        using SuccessCallback =
            std::function<void(TreatmentList)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit TreatmentRepository(
            drogon::orm::DbClientPtr databaseClient
        );

        void findAll(
            std::optional<int> patientId,
            std::optional<int> doctorId,
            std::string searchTerm,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}