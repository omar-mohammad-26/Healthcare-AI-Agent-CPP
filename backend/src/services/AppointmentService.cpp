#include "services/AppointmentService.h"

#include <cctype>
#include <memory>
#include <string>
#include <utility>

namespace
{
bool isDigitAt(
    const std::string &value,
    const std::size_t position
)
{
    return position < value.size() &&
           std::isdigit(
               static_cast<unsigned char>(
                   value[position]
               )
           );
}

int parseTwoDigits(
    const std::string &value,
    const std::size_t position
)
{
    return
        (value[position] - '0') * 10 +
        (value[position + 1] - '0');
}

int parseFourDigits(
    const std::string &value,
    const std::size_t position
)
{
    return
        (value[position] - '0') * 1000 +
        (value[position + 1] - '0') * 100 +
        (value[position + 2] - '0') * 10 +
        (value[position + 3] - '0');
}

bool isLeapYear(
    const int year
)
{
    return
        year % 400 == 0 ||
        (year % 4 == 0 && year % 100 != 0);
}

int daysInMonth(
    const int year,
    const int month
)
{
    switch (month)
    {
        case 2:
            return isLeapYear(year) ? 29 : 28;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        default:
            return 31;
    }
}

bool isValidAppointmentDatetime(
    const std::string &value
)
{
    if (value.size() != 20 &&
        value.size() != 25)
    {
        return false;
    }

    const std::size_t digitPositions[] =
    {
        0, 1, 2, 3,
        5, 6,
        8, 9,
        11, 12,
        14, 15,
        17, 18
    };

    for (const auto position : digitPositions)
    {
        if (!isDigitAt(value, position))
        {
            return false;
        }
    }

    if (value[4] != '-' ||
        value[7] != '-' ||
        value[10] != 'T' ||
        value[13] != ':' ||
        value[16] != ':')
    {
        return false;
    }

    if (value.size() == 20)
    {
        if (value[19] != 'Z')
        {
            return false;
        }
    }
    else
    {
        if (value[19] != '+' &&
            value[19] != '-')
        {
            return false;
        }

        if (!isDigitAt(value, 20) ||
            !isDigitAt(value, 21) ||
            value[22] != ':' ||
            !isDigitAt(value, 23) ||
            !isDigitAt(value, 24))
        {
            return false;
        }

        const int timezoneHour =
            parseTwoDigits(value, 20);

        const int timezoneMinute =
            parseTwoDigits(value, 23);

        if (timezoneHour > 14 ||
            timezoneMinute > 59)
        {
            return false;
        }
    }

    const int year =
        parseFourDigits(value, 0);

    const int month =
        parseTwoDigits(value, 5);

    const int day =
        parseTwoDigits(value, 8);

    const int hour =
        parseTwoDigits(value, 11);

    const int minute =
        parseTwoDigits(value, 14);

    const int second =
        parseTwoDigits(value, 17);

    if (year < 2000 ||
        year > 2200 ||
        month < 1 ||
        month > 12 ||
        day < 1 ||
        day > daysInMonth(year, month) ||
        hour > 23 ||
        minute > 59 ||
        second > 59)
    {
        return false;
    }

    return true;
}

Json::Value appointmentToJson(
    const healthcare::models::Appointment &appointment
)
{
    Json::Value appointmentJson;

    appointmentJson["id"] =
        appointment.id;

    appointmentJson["patient_id"] =
        appointment.patientId;

    appointmentJson["patient_name"] =
        appointment.patientName;

    appointmentJson["doctor_id"] =
        appointment.doctorId;

    appointmentJson["doctor_name"] =
        appointment.doctorName;

    appointmentJson["doctor_specialty"] =
        appointment.doctorSpecialty;

    appointmentJson["appointment_datetime"] =
        appointment.appointmentDatetime;

    appointmentJson["status"] =
        appointment.status;

    appointmentJson["created_at"] =
        appointment.createdAt;

    return appointmentJson;
}
}

namespace healthcare::services
{
AppointmentService::AppointmentService(
    drogon::orm::DbClientPtr databaseClient
)
    : appointmentRepository_(
          std::move(databaseClient)
      )
{
}

void AppointmentService::listAppointments(
    std::string status,
    std::optional<int> doctorId,
    std::optional<int> patientId,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    appointmentRepository_.findAll(
        std::move(status),
        doctorId,
        patientId,
        [
            successCallback =
                std::move(successCallback)
        ](
            healthcare::repositories::
                AppointmentRepository::AppointmentList
                    appointments
        ) mutable
        {
            Json::Value appointmentArray(
                Json::arrayValue
            );

            for (const auto &appointment : appointments)
            {
                appointmentArray.append(
                    appointmentToJson(appointment)
                );
            }

            Json::Value responseBody;

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    appointments.size()
                );

            responseBody["data"] =
                std::move(appointmentArray);

            successCallback(
                std::move(responseBody)
            );
        },
        std::move(errorCallback)
    );
}

void AppointmentService::createAppointment(
    healthcare::models::CreateAppointmentInput input,
    SuccessCallback successCallback,
    MutationErrorCallback errorCallback) const
{
    if (input.patientId <= 0)
    {
        errorCallback(
            {
                AppointmentServiceErrorType::Validation,
                "patient_id",
                "Patient ID must be a positive integer."
            }
        );

        return;
    }

    if (input.doctorId <= 0)
    {
        errorCallback(
            {
                AppointmentServiceErrorType::Validation,
                "doctor_id",
                "Doctor ID must be a positive integer."
            }
        );

        return;
    }

    if (!isValidAppointmentDatetime(
            input.appointmentDatetime
        ))
    {
        errorCallback(
            {
                AppointmentServiceErrorType::Validation,
                "appointment_datetime",
                "Appointment datetime must use ISO 8601 format, such as 2026-08-10T09:00:00Z."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<MutationErrorCallback >(
            std::move(errorCallback)
        );

    appointmentRepository_.create(
        std::move(input),
        [
            sharedSuccessCallback,
            sharedErrorCallback
        ](
            healthcare::models::
                CreateAppointmentResult result
        )
        {
            switch (result.outcome)
            {
                case healthcare::models::
                    CreateAppointmentOutcome::Created:
                {
                    Json::Value responseBody;
                    responseBody["status"] =
                        "created";

                    responseBody["data"] =
                        appointmentToJson(
                            result.appointment.value()
                        );

                    (*sharedSuccessCallback)(
                        std::move(responseBody)
                    );

                    return;
                }

                case healthcare::models::
                    CreateAppointmentOutcome::
                        PatientNotFound:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                NotFound,
                            "patient_id",
                            "The requested patient does not exist."
                        }
                    );
                    return;

                case healthcare::models::
                    CreateAppointmentOutcome::
                        DoctorNotFound:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                NotFound,
                            "doctor_id",
                            "The requested doctor does not exist."
                        }
                    );
                    return;

                case healthcare::models::
                    CreateAppointmentOutcome::
                        PastDatetime:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                Validation,
                            "appointment_datetime",
                            "Appointment datetime must be in the future."
                        }
                    );
                    return;

                case healthcare::models::
                    CreateAppointmentOutcome::Conflict:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                Conflict,
                            "appointment_datetime",
                            "The doctor or patient already has an active appointment at this time."
                        }
                    );
                    return;
            }
        },
        [
            sharedErrorCallback
        ](
            std::string errorMessage
        )
        {
            (*sharedErrorCallback)(
                {
                    AppointmentServiceErrorType::Database,
                    "",
                    std::move(errorMessage)
                }
            );
        }
    );
}
    void AppointmentService::cancelAppointment(
    const int appointmentId,
    SuccessCallback successCallback,
    MutationErrorCallback errorCallback
) const
{
    if (appointmentId <= 0)
    {
        errorCallback(
            {
                AppointmentServiceErrorType::Validation,
                "appointment_id",
                "Appointment ID must be a positive integer."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<MutationErrorCallback>(
            std::move(errorCallback)
        );

    appointmentRepository_.cancel(
        appointmentId,
        [
            sharedSuccessCallback,
            sharedErrorCallback
        ](
            healthcare::models::
                CancelAppointmentResult result
        )
        {
            switch (result.outcome)
            {
                case healthcare::models::
                    CancelAppointmentOutcome::Cancelled:
                {
                    Json::Value responseBody;

                    responseBody["status"] =
                        "cancelled";

                    responseBody["data"] =
                        appointmentToJson(
                            result.appointment.value()
                        );

                    (*sharedSuccessCallback)(
                        std::move(responseBody)
                    );

                    return;
                }

                case healthcare::models::
                    CancelAppointmentOutcome::NotFound:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                NotFound,
                            "appointment_id",
                            "The requested appointment does not exist."
                        }
                    );
                    return;

                case healthcare::models::
                    CancelAppointmentOutcome::
                        AlreadyCancelled:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                Conflict,
                            "appointment_id",
                            "The appointment is already cancelled."
                        }
                    );
                    return;

                case healthcare::models::
                    CancelAppointmentOutcome::Completed:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                Conflict,
                            "appointment_id",
                            "A completed appointment cannot be cancelled."
                        }
                    );
                    return;

                case healthcare::models::
                    CancelAppointmentOutcome::Conflict:
                    (*sharedErrorCallback)(
                        {
                            AppointmentServiceErrorType::
                                Conflict,
                            "appointment_id",
                            "The appointment could not be cancelled because its status changed."
                        }
                    );
                    return;
            }
        },
        [
            sharedErrorCallback
        ](
            std::string errorMessage
        )
        {
            (*sharedErrorCallback)(
                {
                    AppointmentServiceErrorType::Database,
                    "",
                    std::move(errorMessage)
                }
            );
        }
    );
}
}