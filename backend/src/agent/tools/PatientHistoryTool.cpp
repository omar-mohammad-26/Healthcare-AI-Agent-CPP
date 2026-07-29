#include "agent/tools/PatientHistoryTool.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <memory>
#include <string>
#include <utility>

namespace
{
Json::Value nullableString(
    const drogon::orm::Row &row,
    const std::string &columnName
)
{
    if (row[columnName].isNull())
    {
        return Json::Value(Json::nullValue);
    }

    return row[columnName].as<std::string>();
}

Json::Value patientFromRow(
    const drogon::orm::Row &row
)
{
    Json::Value patient;

    patient["id"] =
        row["patient_id"].as<int>();

    patient["full_name"] =
        row["patient_name"].as<std::string>();

    patient["phone"] =
        nullableString(
            row,
            "patient_phone"
        );

    if (row["patient_age"].isNull())
    {
        patient["age"] =
            Json::Value(Json::nullValue);
    }
    else
    {
        patient["age"] =
            row["patient_age"].as<int>();
    }

    patient["gender"] =
        nullableString(
            row,
            "patient_gender"
        );

    patient["medical_history"] =
        nullableString(
            row,
            "patient_medical_history"
        );

    patient["created_at"] =
        row["patient_created_at"]
            .as<std::string>();

    return patient;
}

Json::Value treatmentFromRow(
    const drogon::orm::Row &row
)
{
    Json::Value treatment;

    treatment["id"] =
        row["treatment_id"].as<int>();

    treatment["doctor_id"] =
        row["treatment_doctor_id"].as<int>();

    treatment["doctor_name"] =
        row["treatment_doctor_name"]
            .as<std::string>();

    treatment["doctor_specialty"] =
        row["treatment_doctor_specialty"]
            .as<std::string>();

    treatment["diagnosis"] =
        row["treatment_diagnosis"]
            .as<std::string>();

    treatment["treatment_plan"] =
        row["treatment_plan"]
            .as<std::string>();

    treatment["created_at"] =
        row["treatment_created_at"]
            .as<std::string>();

    return treatment;
}
}

namespace healthcare::agent::tools
{
PatientHistoryTool::PatientHistoryTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string PatientHistoryTool::name() const
{
    return "patient_history";
}

std::string PatientHistoryTool::description() const
{
    return
        "Returns a fictional patient's profile, stored "
        "medical-history note, and treatment records.";
}

Json::Value PatientHistoryTool::parametersSchema() const
{
    Json::Value patientIdProperty;

    patientIdProperty["type"] =
        "integer";

    patientIdProperty["description"] =
        "The unique ID of the patient.";

    patientIdProperty["minimum"] =
        1;

    Json::Value properties;

    properties["patient_id"] =
        std::move(patientIdProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("patient_id");

    Json::Value schema;

    schema["type"] =
        "object";

    schema["properties"] =
        std::move(properties);

    schema["required"] =
        std::move(required);

    schema["additionalProperties"] =
        false;

    return schema;
}

void PatientHistoryTool::execute(
    const Json::Value &arguments,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!arguments.isObject())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "arguments",
                "Tool arguments must be a JSON object."
            }
        );

        return;
    }

    if (!arguments.isMember("patient_id") ||
        !arguments["patient_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID is required and must be an integer."
            }
        );

        return;
    }

    const int patientId =
        arguments["patient_id"].asInt();

    if (patientId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID must be a positive integer."
            }
        );

        return;
    }

    if (!databaseClient_)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Database,
                "",
                "The PostgreSQL client is not available."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ErrorCallback>(
            std::move(errorCallback)
        );

    databaseClient_->execSqlAsync(
        R"(
            SELECT
                patient.id AS patient_id,
                patient.full_name AS patient_name,
                patient.phone AS patient_phone,
                patient.age AS patient_age,
                patient.gender AS patient_gender,
                patient.medical_history
                    AS patient_medical_history,
                patient.created_at
                    AS patient_created_at,

                treatment.id AS treatment_id,
                treatment.doctor_id
                    AS treatment_doctor_id,
                doctor.full_name
                    AS treatment_doctor_name,
                doctor.specialty
                    AS treatment_doctor_specialty,
                treatment.diagnosis
                    AS treatment_diagnosis,
                treatment.treatment_plan,
                treatment.created_at
                    AS treatment_created_at
            FROM patients AS patient
            LEFT JOIN treatments AS treatment
                ON treatment.patient_id = patient.id
            LEFT JOIN doctors AS doctor
                ON doctor.id = treatment.doctor_id
            WHERE patient.id = CAST($1 AS INTEGER)
            ORDER BY
                treatment.created_at DESC,
                treatment.id DESC
        )",
        [
            sharedSuccessCallback,
            sharedErrorCallback,
            patientId
        ](
            const drogon::orm::Result &result
        )
        {
            if (result.size() == 0)
            {
                (*sharedErrorCallback)(
                    {
                        healthcare::agent::
                            AgentToolErrorType::NotFound,
                        "patient_id",
                        "The requested patient does not exist."
                    }
                );

                return;
            }

            const auto firstRow =
                result[0];

            Json::Value treatments(
                Json::arrayValue
            );

            for (const auto &row : result)
            {
                if (row["treatment_id"].isNull())
                {
                    continue;
                }

                treatments.append(
                    treatmentFromRow(row)
                );
            }

            Json::Value historyResult;

            historyResult["patient"] =
                patientFromRow(firstRow);

            historyResult["treatment_count"] =
                static_cast<Json::UInt64>(
                    treatments.size()
                );

            historyResult["treatments"] =
                std::move(treatments);

            Json::Value toolResult;

            toolResult["tool"] =
                "patient_history";

            toolResult["patient_id"] =
                patientId;

            toolResult["result"] =
                std::move(historyResult);

            (*sharedSuccessCallback)(
                std::move(toolResult)
            );
        },
        [
            sharedErrorCallback
        ](
            const drogon::orm::DrogonDbException
                &exception
        )
        {
            (*sharedErrorCallback)(
                {
                    healthcare::agent::
                        AgentToolErrorType::Database,
                    "",
                    exception.base().what()
                }
            );
        },
        patientId
    );
}
}