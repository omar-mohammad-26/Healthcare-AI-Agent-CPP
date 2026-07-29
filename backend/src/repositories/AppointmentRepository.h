#pragma once

#include "models/Appointment.h"

#include <drogon/orm/DbClient.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace healthcare::repositories
{
    class AppointmentRepository final
    {
    public:
        using AppointmentList =
            std::vector<healthcare::models::Appointment>;

        using SuccessCallback =
            std::function<void(AppointmentList)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        using CreateSuccessCallback =
            std::function<
                void(healthcare::models::CreateAppointmentResult)
            >;

        using CancelSuccessCallback =
            std::function<
                void(healthcare::models::CancelAppointmentResult)
            >;

        explicit AppointmentRepository(
            drogon::orm::DbClientPtr databaseClient
        );

        void findAll(
            std::string status,
            std::optional<int> doctorId,
            std::optional<int> patientId,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void create(
            healthcare::models::CreateAppointmentInput input,
            CreateSuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void cancel(
            int appointmentId,
            CancelSuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}