#include "mb_http_utils.h"

#include "mb_connection_context.h"

#include <stdarg.h>

#if __has_include(<cJSON.h>)
#include <cJSON.h>
#else
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#endif
#endif

namespace mb {

std::tuple<int, const char *, int> memsnprintf(int _result_code, const char *__restrict __format, ...) __THROWNL
{
    va_list args;
    va_start(args, __format);
    auto sz = vsnprintf(nullptr, 0, __format, args) + 1;
    va_end(args);
    char *result = static_cast<char *>(malloc(sz));
    va_start(args, __format);
    vsnprintf(result, sz, __format, args);
    va_end(args);
    return {_result_code, result, sz - 1};
}

std::pair<std::optional<int>, std::string> parse_json_int(cJSON *json, std::string key)
{
    std::optional<int> result;
    std::string error;
    //DEBUG_MSG(upload->upload_data << "\n");

    if(!json)
    {
        error = "101: Corpo da requisição vazio";
    }
    else
    {
        //DEBUG_MSG(key << "\n");
        cJSON *data = cJSON_GetObjectItemCaseSensitive(json, key.c_str());

        if(!data)
        {
            error = "000 - dados não encontrados";
        }
        else
        {
            if(data->type != cJSON_Number)
            {
                error = "Tipo incorreto de campo, esperando int, ou campo não encontrado";
            }
            else
            {
                result = data->valueint;
            }
        }
    }

    return std::make_pair(result, error);
}

std::pair<std::optional<int>, std::string> parse_json_int(connection_context *ctx, std::string key)
{
    std::optional<int> result;
    std::string error;
    auto upload = dynamic_cast<upload_data_context *>(ctx);
    auto *json = cJSON_Parse(upload->upload_data.c_str());
    //DEBUG_MSG(upload->upload_data << "\n");

    if(!json)
    {
        error = "101: Corpo da requisição vazio";
    }
    else
    {
        //DEBUG_MSG(key << "\n");
        cJSON *data = cJSON_GetObjectItemCaseSensitive(json, key.c_str());

        if(!data)
        {
            error = "000 - dados não encontrados";
        }
        else
        {
            if(data->type != cJSON_Number)
            {
                error = "Tipo incorreto de campo, esperando int, ou campo não encontrado";
            }
            else
            {
                result = data->valueint;
            }
        }
    }

    return std::make_pair(result, error);
}

std::pair<std::optional<std::string>, std::string> parse_json(connection_context *ctx, std::string key)
{
    std::optional<std::string> result;
    std::string error;
    auto upload = dynamic_cast<upload_data_context *>(ctx);
    auto json = cJSON_Parse(upload->upload_data.c_str());

    if(!json)
    {
        error = "101: Corpo da requisição vazio";
    }
    else
    {
        auto data = cJSON_GetObjectItemCaseSensitive(json, key.c_str());

        if(data == nullptr or data->type != cJSON_String)
        {
            error = "Tipo incorreto de campo, esperando string, ou campo não encontrado";
        }
        else
        {
            result = data->valuestring;
            DEBUG_MSG(TASK, DEBUG, "result = " << data->valuestring << "\n");

            if(result->size() == 0)
            {
                result = {};
                error = "102: " + key + " não encontrado";
            }
        }
    }

    return std::make_pair(result, error);
}

}
