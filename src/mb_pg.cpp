#include "mb_pg.h"
#include "common/mb_globals.h"

#include <libpq-fe.h>

#include <poll.h>
#include <string.h>
#include <cstdarg>
#include <cstdlib>

namespace mb {

PQConnection::PQConnection()
{
}

PQConnection::~PQConnection()
{
    if(m_conn)
    {
        do
        {
            poll();
        }
        while(m_need_flush || (!m_ready) || PQisBusy(m_conn) == 1);

        PQfinish(m_conn);
    }
}

PQConnection::operator bool() const
{
    return m_conn && PQstatus(m_conn) == CONNECTION_OK;
}

bool PQConnection::connect(const char *_params)
{
    m_conn = PQconnectdb(_params);
    m_ready = operator bool();

    while(true)
        switch(PQconnectPoll(m_conn))
        {
            case PGRES_POLLING_FAILED:
                DEBUG_MSG(PQerrorMessage(m_conn) << endl);
                return false;

            case PGRES_POLLING_READING:
            case PGRES_POLLING_WRITING:
                continue;

            case PGRES_POLLING_OK:
                DEBUG_MSG("Connected!\n");
#if __cplusplus >= 201703L
                [[fallthrough]];
#endif

            case PGRES_POLLING_ACTIVE:
                PQsetnonblocking(m_conn, 1);
                return m_ready;
        }
}

const char *PQConnection::error_message()
{
    return PQerrorMessage(m_conn);
}

void PQConnection::flush()
{
    switch(PQflush(m_conn))
    {
        case -1:
            DEBUG_MSG("Error Flushing Query: " << PQerrorMessage(m_conn) << endl);
            break;

        case 0:
            m_need_flush = false;
            break;

        case 1:
            m_need_flush = true;
            break;
    }
}

void PQConnection::poll()
{
    struct pollfd fds;
    fds.fd = PQsocket(m_conn);
    fds.events = POLLIN | POLLOUT;
    fds.revents = 0;
    auto ready = ::poll(&fds, 1, 1);

    switch(ready)
    {
        case -1:
        {
            DEBUG_MSG("poll failed: " << strerror(errno) << "\n");
            return;
        }

        case 0:
        {
            return;
        }

        default:
        {
            {
                if(fds.revents & POLLOUT)
                {
                    flush();

                    if((fds.revents & POLLIN) == 0)
                    {
                        m_ready = PQisBusy(m_conn) != 1;
                    }
                }

                if(fds.revents & POLLIN)
                {
                    if(PQconsumeInput(m_conn) == 0)
                    {
                        DEBUG_MSG("Consume failed: " << PQerrorMessage(m_conn) << endl);
                        return;
                    }

                    while(PQisBusy(m_conn) == 0)
                    {
                        auto res = PQgetResult(m_conn);

                        if(res == nullptr)
                        {
                            m_ready = true;
                            break;
                        }
                        else
                        {
                            switch(auto status = PQresultStatus(res))
                            {
                                case PGRES_EMPTY_QUERY:         /* empty query string was executed */
                                {
                                    DEBUG_MSG(PQresStatus(status) << endl);
                                    break;
                                }

                                case PGRES_COMMAND_OK:          /* a query command that doesn't return
* anything was executed properly by the
* backend */
                                    break;

                                case PGRES_TUPLES_OK:           /* a query command that returns tuples was
* executed properly by the backend, PGresult
* contains the result tuples */
                                    PQconsumeInput(m_conn);

                                    if(m_exec_consumer)
                                    {
                                        Result_Set results;
                                        results.reserve(PQntuples(res));
                                        int row_num = 0;

                                        do
                                        {
                                            Row row;
                                            auto cols = PQnfields(res);
                                            row.reserve(cols);

                                            for(int i = 0; i < cols; i++)
                                            {
                                                row.push_back(PQgetvalue(res, row_num, i));
                                            }

                                            results.push_back(std::move(row));
                                            row_num++;
                                            PQclear(res);
                                        }
                                        while((res = PQgetResult(m_conn)) != nullptr);

                                        m_exec_consumer(std::move(results));
                                    }

                                    break;

                                case PGRES_COPY_OUT:            /* Copy Out data transfer in progress */
                                    break;

                                case PGRES_COPY_IN:             /* Copy In data transfer in progress */
                                    break;

                                case PGRES_BAD_RESPONSE:        /* an unexpected response was recv'd from the
* backend */
                                {
                                    DEBUG_MSG(PQresStatus(status) << endl);
                                    break;
                                }

                                case PGRES_NONFATAL_ERROR:      /* notice or warning message */
                                {
                                    DEBUG_MSG(PQresultVerboseErrorMessage(res, PQERRORS_VERBOSE, PQSHOW_CONTEXT_ERRORS) << endl);
                                    break;
                                }

                                case PGRES_FATAL_ERROR:         /* query failed */
                                {
                                    DEBUG_MSG(PQresultVerboseErrorMessage(res, PQERRORS_VERBOSE, PQSHOW_CONTEXT_ERRORS) << endl);
                                    break;
                                }

                                case PGRES_COPY_BOTH:           /* Copy In/Out data transfer in progress */
                                    break;

                                case PGRES_SINGLE_TUPLE:        /* single tuple from larger resultset */
                                    break;

                                case PGRES_PIPELINE_SYNC:       /* pipeline synchronization point */
                                    break;

                                case PGRES_PIPELINE_ABORTED:    /* Command didn't run because of an abort
* earlier in a pipeline */
                                {
                                    DEBUG_MSG(PQresultVerboseErrorMessage(res, PQERRORS_VERBOSE, PQSHOW_CONTEXT_ERRORS) << endl);
                                    break;
                                }
                            }

                            PQclear(res);
                        }
                    }//while
                } //if
            } //for
            break;
        }//default
    }//switch
}

void PQConnection::async_exec(std::string _sql)
{
    while(PQisBusy(m_conn))
    {
        poll();
    }

    auto ret = PQsendQuery(m_conn, _sql.c_str());
    poll();

    if(ret != 1)
    {
        DEBUG_MSG(error_message());
        mb_assert(false);
    }
}

void PQConnection::async_exec(const char *_sql...)
{
    mb_assert(_sql && strlen(_sql) > 4);
    std::va_list args;
    va_start(args, _sql);
    char *buffer { nullptr };
    size_t sz = vsnprintf(buffer, 0, _sql, args) + 1;
    va_end(args);
    va_start(args, _sql);
    buffer = static_cast<char *>(malloc(sz + 1));
    vsnprintf(buffer, sz, _sql, args);
    async_exec({ buffer, sz });
    free(buffer);
}

std::string PQConnection::escape_literal(const char *_str, size_t _size)
{
    if(_size == 0)
    {
        _size = strlen(_str);
    }

    auto new_str { PQescapeLiteral(m_conn, _str, _size) };
    std::string result { new_str };
    PQfreemem(new_str);
    return result;
}

void PQStatement::execute(const std::stringstream &_sql)
{
    m_result = PQexec(m_conn.conn(), _sql.str().c_str());
    check_error();
}

void PQStatement::execute(const char *_sql...)
{
    mb_assert(_sql && strlen(_sql) > 4);
    clear();
    std::va_list args;
    va_start(args, _sql);
    char *buffer { nullptr };
    size_t sz = vsnprintf(buffer, 0, _sql, args) + 1;
    va_end(args);
    va_start(args, _sql);
    buffer = static_cast<char *>(malloc(sz + 1));
    vsnprintf(buffer, sz, _sql, args);
    m_result = PQexec(m_conn.conn(), buffer);
    free(buffer);
    check_error();
}

void PQStatement::check_error()
{
    switch(status())
    {
        case PGRES_COMMAND_OK:
        case PGRES_TUPLES_OK:
        case PGRES_SINGLE_TUPLE:
            break;

        default:
            DEBUG_MSG(error_message());
            mb_assert(false);
    }
}

void PQStatement::clear()
{
    if(m_result)
    {
        PQclear(m_result);
    }
}

int PQStatement::status() const
{
    return PQresultStatus(m_result);
}

PQStatement::operator bool() const
{
    const auto s = status();
    return s == PGRES_COMMAND_OK || s == PGRES_TUPLES_OK;
}

int PQStatement::size() const
{
    return PQntuples(m_result);
}

const char *PQStatement::get_value(int row, int col) const
{
    return PQgetvalue(m_result, row, col);
}

bool PQStatement::get_is_null(int row, int col) const
{
    return PQgetisnull(m_result, row, col);
}

PQTransaction::~PQTransaction()
{
    if(status() == PQTRANS_INTRANS)
    {
        m_stmt.execute("rollback");
    }
}

int PQTransaction::status()
{
    return PQtransactionStatus(m_stmt.conn().conn());
}

};
