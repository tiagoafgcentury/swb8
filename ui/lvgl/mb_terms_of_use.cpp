#include "mb_terms_of_use.h"

#include "common/mb_globals.h"
#include "common/mb_types.h"

#include "hal/mb_system.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task.h"

#include <filesystem>
#include <unistd.h>

namespace mb {

void Terms_File::accept_terms_of_use()
{
    if (access(MBGUI_TERMS_CONDITIONS_DATE_FILE, F_OK) != 0)
    {
        Task::post_event_save_terms_of_use();
    }
}

Terms_File::App_Terms_of_Use::App_Terms_of_Use():
    version{std::string(tr(__revisao)) + " 1.0"},
    text{"Considerando que:\n\t- USUÁRIO refere-se à pessoa que adquiriu este receptor de satélite e irá utilizá-lo.\n\t- RECEPTOR refere-se a este receptor de satélite que foi adquirido pelo USUÁRIO, identificado pelos números CAID: %s e SCUA: %s.\n\t- FABRICANTE refere-se à empresa que produziu este receptor de satélite (SAT BRAS INDÚSTRIA ELETRÔNICA DA AMAZÔNIA LTDA., CNPJ/MF sob o nº 03.521.296/0001-84).\n\t- SATÉLITE refere-se ao Satélite Sky B1, cuja operação é gerida exclusivamente pela Sky Brasil.\n\t- PORTAL refere-se ao Portal de Ativação de receptores cujo endereço eletrônico é: www.novaparabolica.com, sendo a operação gerida exclusivamente pela Sky Brasil.\n\t- OPERADORA é a Sky Brasil, detentora e gestora do sinal, operação e comercialização de serviço de pacotes de canais pagos.\n\t- INSTALADOR é a pessoa responsável pela configuração e ativação do RECEPTOR no PORTAL, podendo se tratar do próprio USUÁRIO, caso assim deseje.\n\t- TERMO refere-se aos Termos de Uso e Condições que aqui constam.\nEste TERMO aplica-se entre FABRICANTE e USUÁRIO e, ao aceitá-lo, o USUÁRIO declara estar ciente que:\n1) A seleção do SATÉLITE realizada pelo INSTALADOR durante o processo de configuração do RECEPTOR atende a solicitação do USUÁRIO.\n\n2) Uma vez selecionado o SATÉLITE, a ativação do RECEPTOR será feita exclusivamente via PORTAL.\n\n3) O PORTAL oferece canais gratuitos e pagos, sendo que são gerenciados exclusivamente pela OPERADORA.\n\n4) Realizada a ativação, o RECEPTOR sintonizará inicialmente somente os canais gratuitos, cujos critérios de regionalização são divulgados no PORTAL.\n\n5) A critério exclusivo do USUÁRIO, este poderá adquirir, por tempo determinado, pacotes de canais pagos gerenciados exclusivamente pela OPERADORA, os quais passarão a ser sintonizados pelo RECEPTOR juntamente com os canais gratuitos, nos moldes das instruções do PORTAL.\n\n6). Para que o RECEPTOR possa sintonizar os pacotes de canais pagos, deverá aguardar um \"comando\" via SATÉLITE que será enviado pela OPERADORA, após a aprovação da compra.\n\n7) Ao final do período do pacote de canais pagos contratado pelo USUÁRIO, a OPERADORA enviará outro \"comando\" via SATÉLITE ao RECEPTOR para que o referido pare de sintonizá-los, ficando então disponíveis novamente aos USUÁRIOS somente os canais gratuitos.\n\n8) O sistema de venda de pacotes, pagamentos, ativação dos canais e sua regularidade, bem como o tratamento de dados (Lei n° 13.709/2018), é gerenciado exclusivamente pela OPERADORA, não tendo a FABRICANTE qualquer participação nas tratativas com USUÁRIO.\n\n9) A FABRICANTE não possui responsabilidade sobre a transmissão dos canais digitais (gratuitos ou pagos) do SATÉLITE, tampouco sobre a criação, alteração, suspensão, disponibilização ou exclusão de pacotes e funcionalidades, indisponibilidade de qualquer ordem ou descontinuação da transmissão por parte da OPERADORA, de modo que, na eventualidade de inconsistências, o USUÁRIO deverá entrar em contato diretamente com o suporte oficial da OPERADORA.\n\n10) A FABRICANTE não se responsabiliza por erros técnicos na instalação ou manutenção do RECEPTOR realizada por terceiros não pertencentes à Rede Credenciada nem pelo conteúdo dos pacotes da OPERADORA.\n\n11) A responsabilidade da FABRICANTE limita-se ao funcionamento do RECEPTOR, desde que o SATÉLITE mantenha as configurações de transmissão em conformidade com as suas especificações técnicas, sendo que o sinal e funcionalidades são geridos exclusivamente pela OPERADORA (especialmente ao que se refere aos canais pagos).\n\n12) A FABRICANTE não se responsabiliza pela ausência de sinal, total ou parcial, advinda do SATÉLITE, em razão de condições climáticas adversas.\n\n13) A FABRICANTE se reserva ao direito de a seu exclusivo critério e a qualquer tempo, atualizar os Termos de Uso, integral ou parcialmente, passando tais mudanças a vigorar de forma automática a partir da disponibilização. A continuidade da utilização pelo USUÁRIO, será considerada como aceite tácito, sem prejuízo da FABRICANTE solicitar seu consentimento expresso, se o caso.\n\n14) A FABRICANTE poderá impor limites à determinadas funcionalidades e serviços, podendo encerrar a autorização e licenças concedidas a terceiros a qualquer tempo, se reservando ainda à possibilidade de (i) estipular regras e procedimentos, incluindo a investigação de potenciais violações aqui previstas; (ii) detecção e prevenção das referidas e (iii) alinhamentos técnicos e (iv) proteção de direitos, propriedades intelectuais e físicas e segurança."},
    satellite{"Sky B1"}
{
}

Terms_File::App_Terms_of_Use Terms_File::get_terms_of_use()
{
    App_Terms_of_Use result;
    std::string content = cat(MBGUI_TERMS_CONDITIONS_JSON_FILE);

    if (content.empty())
    {
        DEBUG_MSG(OSD, ERROR, "Error opening file " << MBGUI_TERMS_CONDITIONS_JSON_FILE << "\n");
    }

    auto json = cJSON_Parse(content.c_str());

    if (json == nullptr)
    {
        DEBUG_MSG(OSD, ERROR, "Error parsing " << MBGUI_TERMS_CONDITIONS_JSON_FILE << "\n");
    }
    else
    {
        auto terms = cJSON_GetObjectItemCaseSensitive(json, "terms");

        if (not cJSON_IsArray(terms))
        {
            DEBUG_MSG(OSD, ERROR, "Error parsing terms\n");
            goto EXIT_RESULT;
        }

        if (auto satellite_json = cJSON_GetObjectItemCaseSensitive(terms, "satellite"); satellite_json)
        {
            result.satellite = satellite_json->valuestring;
        }
        else
        {
            DEBUG_MSG(OSD, ERROR, "Error parsing satellite\n");
            goto EXIT_RESULT;
        }

        if (auto version_json = cJSON_GetObjectItemCaseSensitive(terms, "version"); version_json)
        {
            result.version = version_json->valuestring;
        }
        else
        {
            DEBUG_MSG(OSD, ERROR, "Error parsing version\n");
            goto EXIT_RESULT;
        }

        if (auto text_json = cJSON_GetObjectItemCaseSensitive(terms, "text"); text_json)
        {
            result.text = text_json->valuestring;
        }
        else
        {
            DEBUG_MSG(OSD, ERROR, "Error parsing text\n");
            goto EXIT_RESULT;
        }
    }

EXIT_RESULT:
    return result;
}

bool Terms_File::terms_has_been_accepted()
{
    return std::filesystem::exists(MBGUI_TERMS_CONDITIONS_DATE_FILE);
}

}
