#pragma once

#ifdef MBGUI_SAT_MONITOR

#include <sstream>
#include <functional>
#include <vector>

typedef struct pg_conn PGconn;
struct pg_result;

namespace mb {

class PQConnection
{
public:
    typedef std::vector<std::string> Row;
    typedef std::vector<Row> Result_Set;
    typedef std::function<void(Result_Set _result)> Exec_Consumer;

private:
    PGconn *m_conn = nullptr;

    bool m_need_flush = false;
    bool m_ready = false;

    Exec_Consumer m_exec_consumer;
public:
    PQConnection();
    PQConnection(const char *_params)
    {
        connect(_params);
    }

    ~PQConnection();

    bool connect(const char *_params);

    auto conn()
    {
        return m_conn;
    }

    operator bool() const;

    const char *error_message();

    void async_exec(std::string _sql);
    void async_exec(const char *_sql...) __attribute__((format(printf, 2, 3)));


    void set_exec_consumer(Exec_Consumer _consumer)
    {
        m_exec_consumer = _consumer;
    }

    std::string escape_literal(const char *_str, size_t _size);
    std::string escape_literal(const std::string &_str)
    {
        return escape_literal(_str.c_str(), _str.size());
    }

    void poll();
    void flush();
};

class PQStatement
{
private:
    PQConnection &m_conn;
    pg_result *m_result { nullptr };

    void check_error();

public:
    PQStatement(PQConnection &_conn):
        m_conn(_conn)
    {}

    auto conn()
    {
        return m_conn;
    }

    void execute(const std::stringstream &_sql);
    void execute(const char *_sql...) __attribute__((format(printf, 2, 3)));

    ~PQStatement()
    {
        clear();
    }

    void clear();

    int status() const;

    operator bool() const;

    const char *error_message() const
    {
        return m_conn.error_message();
    }

    int size() const;

    const char *get_value(int row, int col) const;

    bool get_is_null(int row, int col) const;

    std::string escape_literal(const char *_str, size_t _size)
    {
        return m_conn.escape_literal(_str, _size);
    }

    std::string escape_literal(const std::string &_str)
    {
        return m_conn.escape_literal(_str.c_str(), _str.size());
    }
};

class PQTransaction
{
private:
    PQStatement m_stmt;

public:
    PQTransaction(PQConnection &_conn):
        m_stmt(_conn)
    {
        m_stmt.execute("begin");
    }

    int status();

    ~PQTransaction();

    void commit()
    {
        m_stmt.execute("commit");
    }
};

} // namespace mb

#endif // MBGUI_SAT_MONITOR
