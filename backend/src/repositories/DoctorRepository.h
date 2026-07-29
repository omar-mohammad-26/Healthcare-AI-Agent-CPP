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

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}