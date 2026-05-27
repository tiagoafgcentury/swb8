#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <istream>

namespace mb {

class static_string
{
private:
    std::shared_ptr<std::string> m_p;

public:
    static_string()
    {
    }

    static_string(const static_string &_other):
        m_p(_other.m_p)
    {
    }

    static_string(static_string &&_other):
        m_p(std::move(_other.m_p))
    {
    }

    static_string &operator=(const static_string &_other)
    {
        m_p = _other.m_p;
        return *this;
    }

    static_string(const std::string &_string):
        m_p(std::make_shared<std::string>(_string))
    {
    }

    static_string(std::string &&_string):
        m_p(std::make_shared<std::string>(std::move(_string)))
    {
    }

    static_string(const char *_string):
        m_p(std::make_shared<std::string>(_string))
    {
    }

    static_string(const std::string_view &_string):
        m_p(std::make_shared<std::string>(_string.data()))
    {
    }

    const char *data() const
    {
        if(m_p)
        {
            return m_p->data();
        }

        return nullptr;
    }

    std::size_t size() const
    {
        if(m_p)
        {
            return m_p->size();
        }

        return 0;
    }

    bool empty() const
    {
        if(m_p)
        {
            return m_p->empty();
        }

        return true;
    }
};

// Enable or disable globally
#ifdef MB_STATIC_STRING
typedef ::mb::static_string Static_String;
#else
typedef std::string Static_String;
#endif

inline std::istream &operator>>(std::istream &_stream, static_string &_string)
{
    std::string str;
    char ch;

    // Find first valid char
    while(_stream.get(ch) && isspace(ch))
    {
        // No-op
    }

    if(not isspace(ch))
    {
        str += ch;
    }

    while(_stream.get(ch) and not isspace(ch))
    {
        str += ch;
    }

    _string = static_string(std::move(str));
    return _stream;
}

inline std::ostream &operator<<(std::ostream &_stream, const static_string &_string)
{
    if(not _string.empty())
    {
        _stream << _string.data();
    }

    return _stream;
}

} // namespace mb
