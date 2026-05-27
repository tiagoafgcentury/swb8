#pragma once

#ifndef NDEBUG
#define TABLE_REQUIRE_NAME_TYPE_PARAM const char* _table_name [[maybe_unused]],
#define TABLE_REQUIRE_NAME_PARAM _table_name,
#define TABLE_REQUIRE_NAME(NAME) # NAME ,
#else
#define TABLE_REQUIRE_NAME_PARAM
#define TABLE_REQUIRE_NAME_TYPE_PARAM
#define TABLE_REQUIRE_NAME(NAME)
#endif
