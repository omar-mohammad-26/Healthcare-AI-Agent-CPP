#pragma once

#include "repositories/AppointmentRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <optional>
#include <string>

namespace healthcare::services
{
    class AppointmentService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit AppointmentService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listAppointments(
            std::string status,
            std::optional<int> doctorId,
            std::optional<int> patientId,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::AppointmentRepository
            appointmentRepository_;
    };
}