#include "config/DatabaseConfig.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
Json::Value loadApplicationConfiguration(
    const std::filesystem::path &configPath
)
{
    std::ifstream configFile(
        configPath,
        std::ios::binary
    );

    if (!configFile.is_open())
    {
        throw std::runtime_error(
            "Unable to open Drogon configuration file: " +
            configPath.string()
        );
    }

    Json::CharReaderBuilder readerBuilder;

    readerBuilder["collectComments"] =
        false;

    Json::Value configuration;
    std::string parsingErrors;

    if (!Json::parseFromStream(
            readerBuilder,
            configFile,
            &configuration,
            &parsingErrors
        ))
    {
        throw std::runtime_error(
            "Invalid Drogon configuration file: " +
            parsingErrors
        );
    }

    configuration["db_clients"] =
        healthcare::config::
            buildDatabaseClientsConfig();

    return configuration;
}

bool isAllowedFrontendOrigin(
    const std::string &origin
)
{
    return
        origin == "http://localhost:3000" ||
        origin == "http://127.0.0.1:3000" ||
        origin == "http://localhost:5173" ||
        origin == "http://127.0.0.1:5173";
}

void addCorsHeaders(
    const drogon::HttpRequestPtr &request,
    const drogon::HttpResponsePtr &response
)
{
    if (!request ||
        !response)
    {
        return;
    }

    const std::string origin =
        request->getHeader("origin");

    if (isAllowedFrontendOrigin(origin))
    {
        response->addHeader(
            "Access-Control-Allow-Origin",
            origin
        );

        response->addHeader(
            "Vary",
            "Origin"
        );
    }

    response->addHeader(
        "Access-Control-Allow-Methods",
        "GET, POST, PUT, PATCH, DELETE, OPTIONS"
    );

    response->addHeader(
        "Access-Control-Allow-Headers",
        "Content-Type, Authorization"
    );

    response->addHeader(
        "Access-Control-Max-Age",
        "600"
    );
}

void configureCors()
{
    drogon::app().registerPreRoutingAdvice(
        [](
            const drogon::HttpRequestPtr &request,
            drogon::AdviceCallback &&callback,
            drogon::AdviceChainCallback &&chainCallback
        )
        {
            if (request->method() !=
                drogon::Options)
            {
                chainCallback();

                return;
            }

            auto response =
                drogon::HttpResponse::
                    newHttpResponse();

            response->setStatusCode(
                drogon::k204NoContent
            );

            addCorsHeaders(
                request,
                response
            );

            callback(
                response
            );
        }
    );

    drogon::app().registerPostHandlingAdvice(
        [](
            const drogon::HttpRequestPtr &request,
            const drogon::HttpResponsePtr &response
        )
        {
            addCorsHeaders(
                request,
                response
            );
        }
    );
}
}

int main(
    int,
    char *argv[]
)
{
    try
    {
        const auto executableDirectory =
            std::filesystem::absolute(
                argv[0]
            ).parent_path();

        const auto configPath =
            executableDirectory /
            "config.json";

        if (!std::filesystem::exists(
                configPath
            ))
        {
            throw std::runtime_error(
                "Drogon configuration file was not found: " +
                configPath.string()
            );
        }

        auto configuration =
            loadApplicationConfiguration(
                configPath
            );

        drogon::app().loadConfigJson(
            std::move(configuration)
        );

        configureCors();

        drogon::app().run();
    }
    catch (const std::exception &exception)
    {
        std::cerr
            << "Failed to start Healthcare AI Agent: "
            << exception.what()
            << '\n';

        return 1;
    }

    return 0;
}