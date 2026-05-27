#pragma once

#include <atomic>

extern std::atomic<bool> g_mbgui_keep_running;
extern std::atomic<bool> g_mbgui_reboot_after_exit;
extern std::atomic<bool> g_mbgui_restart_on_exit;
extern std::atomic<bool> g_mbgui_do_factory_reset;
extern std::atomic<bool> g_mbgui_pause_low_priority_tasks;
