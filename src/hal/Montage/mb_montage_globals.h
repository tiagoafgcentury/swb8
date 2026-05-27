#pragma once

#define MT_EXEC(fn) \
    do                                                                                                           \
    {                                                                                                            \
        auto ret = fn;                                                                                           \
        if (MT_SUCCESS != ret)                                                                                   \
        {                                                                                                        \
            using namespace std;                                                                                 \
            cerr << file_name(__FILE__) << ":" << dec << __LINE__ << " FAILED: " # fn " failed: " << mt_get_error_msg(ret) << endl;  \
        }                                                                                                        \
        mb_assert(MT_SUCCESS == ret);                                                                               \
    }                                                                                                            \
    while(false)
