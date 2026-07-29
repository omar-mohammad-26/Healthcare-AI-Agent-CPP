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

        using ScheduleSuccessCallback =
            std::function<
                void(healthcare::models::DoctorScheduleResult)
            >;

        using AvailabilitySuccessCallback =
            std::function<
                void(healthcare::models::DoctorAvailabilityResult)
            >;

        explicit DoctorRepository(
            drogon::orm::DbClientPtr databaseClient
        );

        void findAll(
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void findSchedule(
            int doctorId,
            std::string date,
            ScheduleSuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void findAvailableSlots(
            int doctorId,
            std::string date,
            AvailabilitySuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}