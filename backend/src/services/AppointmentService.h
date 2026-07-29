#pragma once

#include "repositories/AppointmentRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <optional>
#include <string>

namespace healthcare::services
{
    enum class AppointmentServiceErrorType
    {
        Validation,
        NotFound,
        Conflict,
        Database
    };

    struct AppointmentServiceError
    {
        AppointmentServiceErrorType type;
        std::string field;
        std::string message;
    };

    class AppointmentService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        using CreateErrorCallback =
            std::function<void(AppointmentServiceError)>;

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

        void createAppointment(
            healthcare::models::CreateAppointmentInput input,
            SuccessCallback successCallback,
            CreateErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::AppointmentRepository
            appointmentRepository_;
    };
}