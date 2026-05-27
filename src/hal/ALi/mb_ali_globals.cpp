#include "aui_common.h"
#include "mb_ali_globals.h"

#include "aliplatform/alipltfretcode.h"

namespace mb {

const char *ali_error_msg(int error_code)
{
    switch(error_code)
    {
#define EC(NUM, MSG) case NUM: return # NUM;
            EC(ERROR_NONE, "SUCCESS");
            EC(ERROR_NOMEM, "memory malloc failed");
            EC(ERROR_NOPRIV, "No private data struct");
            EC(ERROR_NOOP, "No operation");
            EC(ERROR_OPEN, "open failed");
            EC(ERROR_NOOPEN, "device not opened");
            EC(ERROR_CFG, "wrong config parameter");
            EC(ERROR_IOCTL, "ioctl failed");
            EC(ERROR_TMOUT, "timeout expired");
            EC(ERROR_WAIT, "wait failed");
            EC(ERROR_NOPORTID, "No portid specified");
            EC(ERROR_NOSTART, "device not started");
            EC(ERROR_READ, "read failed");
            EC(ERROR_WRITE, "write failed");
            EC(ERROR_RESET, "reset failed");
            EC(ERROR_NULLDEV, "no dev data struct");
            EC(ERROR_INVAL, "invalid parameter");
            EC(ERROR_PTCREATE, "Create monitor pthread failed");
            EC(ERROR_PTCANCEL, "Cancel monitor pthread failed");
            EC(ERROR_CHTYPE, "invalid channel type");
            EC(ERROR_NOCH, "no channel");
            EC(ERROR_NOFILTER, "no filter");
            EC(ERROR_CHSTART, "channel start failed");
            EC(ERROR_NOPID, "no pid");
            EC(ERROR_AVSTART, "AV start error");
            EC(ERROR_ADDPID, "adding PID failed");
            EC(ERROR_DELPID, "deleting PID failed");
            EC(ERROR_STATUS, "status error");
            EC(ERROR_CLOSE, "close failed");
            EC(ERROR_MMAP, "mmap failed");
            EC(ERROR_GETMEM, "getting memory failed");
            EC(ERROR_RELMEM, "releasing memory failed");
            EC(ERROR_GETMSG, "get message failed");
            EC(ERROR_BADCB, "invalid callback function");
            EC(ERROR_GETRES, "getting resource failed");
            EC(ERROR_FAILED, "failure");
            EC(ERROR_MKDIR, "failure mkdir");
            EC(ERROR_MKDB, "failure mkdb");
            EC(ERROR_INITDB, "failure initdb");
            EC(ERROR_NODB, "failure nodb");
            EC(ERROR_SEMGET, "failure semget");
            EC(ERROR_SEMSET, "failure semset");
            EC(ERROR_SEMOP, "failure semop");
            EC(ERROR_SEMREAD, "failure semread");
            EC(ERROR_NOSEM, "failure nosem");
            EC(ERROR_OPENDB, "failure opendb");
            EC(ERROR_EXEC, "failure exec");

        default:
            return "Unknown Error";
    }
}

} // namespace mb
