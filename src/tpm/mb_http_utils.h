#pragma once

#include <tuple>
#include <string>
#include <optional>

namespace mb {

struct connection_context;

std::tuple<int, const char *, int> memsnprintf(int _result_code, const char *__restrict __format, ...) __THROWNL __attribute__((__format__(__printf__, 2, 3)));

std::pair<std::optional<int>, std::string> parse_json_int(connection_context *ctx, std::string key);
std::pair<std::optional<std::string>, std::string> parse_json(connection_context *ctx, std::string key);

// Pega o campo VALUE no JSON e declara uma variável com este mesmo nome,
// e se não houver o campo, retorna um erro
#define GET_JSON_VALUE(VALUE)                                       \
    auto VALUE = parse_json(ctx, # VALUE );                         \
    if (!VALUE.first.has_value())                                   \
    {                                                               \
        std::string description = "Campo " # VALUE " não encontrado ou não é do tipo string";     \
        auto tmpl = R"json({"error":"%s","description":"%s"})json"; \
        return memsnprintf(404, tmpl, "101", description.c_str());  \
    }

// Igual o acima, mas para int
#define GET_JSON_INT_VALUE(VALUE)                                   \
    auto VALUE = parse_json_int(ctx, # VALUE );                     \
    if (!VALUE.first.has_value())                                   \
    {                                                               \
        std::string description = "Campo " # VALUE " não encontrado ou não é do tipo int";     \
        auto tmpl = R"json({"error":"%s","description":"%s"})json"; \
        return memsnprintf(404, tmpl, "101", description.c_str());  \
    }

const auto PROJECT_TPM_NAME { "mbgui" };
const auto PROJECT_TPM_DESCRIPTION { "MidiaBox TPM" };
const auto PROJECT_TPM_VERSION_MAJOR { "1" };
const auto PROJECT_TPM_VERSION_MINOR { "4" };
const auto PROJECT_TPM_COMPILE_TIMESTAMP { "20250320-1445" };
const auto PROJECT_TPM_VERSION
{
    std::string("v") + PROJECT_TPM_VERSION_MAJOR + "." + PROJECT_TPM_VERSION_MINOR + "-" + PROJECT_TPM_COMPILE_TIMESTAMP
};

const auto USB_MOUNT_DIR { "/mnt/usb" };
} // namespace mb
