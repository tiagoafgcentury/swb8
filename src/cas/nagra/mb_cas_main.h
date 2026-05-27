#pragma once

#include <atomic>

extern std::atomic<bool> g_mbcas_keep_running;
extern std::atomic<bool> g_mbcas_restart_on_exit;
extern std::atomic<bool> g_mbcas_container_restart;
