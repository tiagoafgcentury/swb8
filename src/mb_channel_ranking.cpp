#include "mb_channel_ranking.h"
#include "tasks/mb_task.h"
#include "common/mb_globals.h"
#include "hal/mb_system.h"
#include "dvb/mb_dvb_utc_mjd.h"

namespace mb {

#ifdef MBGUI_USE_CHANNEL_RANKING
Channel_Ranking Channel_Ranking::ranking_instance;

Channel_Ranking::~Channel_Ranking()
{
}

//
//      Processa canal recebido
// - na primeira varredura apenas salva o service_id e a hora de sintonia
// - calcula o tempo decorrido em minutos
// - salva o valor no mapa
//
void Channel_Ranking::new_channel_ranking(const Service *srv)
{
    // Indica que a rotina está sendo executa pela primeira vez
    static bool first = false;
    DEBUG_MSG("\n\n----> new_channel_ranking()\n");

    if(!srv)
    {
        DEBUG_MSG("Informações do canal sintonizado não foram encontradas\n");
        return;
    }

    do
    {
        DEBUG_MSG("Novo canal: " << srv->service_id() << "/" << srv->name() << "\n");

        // Executa este bloco apenas na primeira vez
        if(first == false)
        {
            first = true;
            DEBUG_MSG("Primeiro canal sintonizado, salvando dados\n");
            break;
        }

        // Início do período
        //const auto start_c = std::chrono::system_clock::to_time_t(previous_start);
        //auto start = std::localtime(&start_c);
        auto start = previous_start;
        DEBUG_MSG("start: " << (int)start.hour() << ":" << (int)start.minute() << ":" << (int)start.second() << "\n");
        // Fim do período
        auto end = UTC_MJD{ System::get_system_time() };
        DEBUG_MSG("end: " << (int)end.hour() << ":" << (int)end.minute() << ":" << (int)end.second() << "\n");
        // Calcula intervalo em minutos
        // *** durante os testes o cálculo será em segundos para facilitar debug
        int interval = (end - start) / 60;
        DEBUG_MSG("Intervalo: " << interval << "\n");

        if(interval > 10 * 24 * 60 * 60)
        {
            DEBUG_MSG("Ignorar intervalos maiores que 10 dias\n");
            break;
        }

        // Busca o canal atual pelo viewer_channel
        auto it = service_ranking.find(previous_viewer_channel);

        if(it == service_ranking.end())
        {
            // Não encontrou o canal no mapa, incluir
            ranking_t result = { previous_name, interval, previous_service_id };
            service_ranking.insert(std::make_pair(previous_viewer_channel, result));
            DEBUG_MSG("Inserindo " << previous_name << " no mapa de resultados\n");
        }
        else
        {
            it->second.minutes += interval;
            DEBUG_MSG("Atualizando tempo de exibição de " << previous_name << "\n");
        }

#ifndef NDEBUG
        // Relatório com resultados
        DEBUG_MSG("\nRanking de canais mais assistidos:\n");

        for(const auto &it : service_ranking)
        {
            DEBUG_MSG(std::setfill(' ') << std::setw(4) << it.first << " - " \
                      << std::setfill(' ') << std::setw(4) << it.second.service_id << " " \
                      << std::setfill(' ') << std::setw(24) << it.second.name << " " \
                      << it.second.minutes << " minutos\n");
        }

        DEBUG_MSG("\n");
#endif // NDEBUG
    }
    while(false);

    // Salva dados para a próxima varredura, tem que estar no fim do programa
    previous_name = srv->name();
    previous_viewer_channel = srv->viewer_channel();
    previous_service_id = srv->service_id();
    previous_start = UTC_MJD{ System::get_system_time() };
}

void Channel_Ranking::save_ranking()
{
    DEBUG_MSG("store_ranking()\n");
}

#else // MBGUI_USE_CHANNEL_RANKING

void Channel_Ranking::new_channel_ranking(const Service *)
{
}

#endif // MBGUI_USE_CHANNEL_RANKING

} // namespace mb
