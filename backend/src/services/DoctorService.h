#pragma once

#include "repositories/DoctorRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::services
{
    enum class DoctorScheduleErrorType
    {
        NotFound,
        Database
    };

    struct DoctorScheduleError
    {
        DoctorScheduleErrorType type;
        std::string message;
    };

    class DoctorService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        using ScheduleErrorCallback =
            std::function<void(DoctorScheduleError)>;

        explicit DoctorService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listDoctors(
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

        void getDoctorSchedule(
            int doctorId,
            std::string date,
            SuccessCallback successCallback,
            ScheduleErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::DoctorRepository
            doctorRepository_;
    };
}