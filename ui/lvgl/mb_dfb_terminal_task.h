#pragma once

#include <string>
#include <queue>
#include <chrono>

#include "../../src/tasks/mb_task.h"


namespace mb {

class DFB_Terminal_Task: public Task
{
private:
    std::queue<std::string> m_message_queue;

    bool m_has_update       { false };
    std::chrono::time_point<std::chrono::system_clock> m_last_clock_update {  };

public:
    DFB_Terminal_Task();
    virtual ~DFB_Terminal_Task();

    void print_message(const std::string &str);

    virtual void process() override;

    void print_menu();
private:
    void load_image(const std::string &_filename);
    void clear();

    void print_clock();
};

};
