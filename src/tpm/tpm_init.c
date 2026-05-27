#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include "aui_ini_config.h"
#include <pthread.h>
#include <sys/prctl.h>

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_kl.h>
#include <aui_dsc.h>
#include <aui_common.h>

//#include "aui_help_print.h"
//#include "aui_test_app_cmd.h"
//#include "aui_test_app.h"

//#include "default_aui_tsi_info.h"
#include "tpm_init.h"
#include "tpm_api.h"

#define AUI_SLEEP(x)    usleep(1000*(x))

static aui_nim_std g_nims_std[AUI_NIM_HANDLER_MAX] = {0};

// Porting the TDS API osal_get_tick.
// Returns time in ms that have elapsed since an arbitrary point in the pass.
#include <sys/times.h>
static unsigned int osal_get_tick()
{
    unsigned int time = 0;
    time   = times(NULL) * (1000 / sysconf(_SC_CLK_TCK));
    return time;
}

// In abnormal situations DVB-C auto QAM mode need about 3s to connect 256 QAM signal.
#define NIM_TIMEOUT_MS (10 * 1000)

// C Band LNB L.O. frequency
#define LO_5150 (5150)
#define LO_5750 (5750)
// Ku Band LNB L.O. frequency
#define LO_9750 (9750)
#define LO_10600 (10600)
// Suppose Ku-Band  low/high band switch point is 11.7GHZ
#define KU_11700 (11700)
#define LO_11400 (11400)
#define LO_10250 (10250)

struct intmap
{
    int a;
    int b;
    char *name;
};

static char *intmap_bname(int b, struct intmap *map)
{
    int i;

    for (i = 0; map[i].name != NULL; i++)
    {
        if (b == map[i].b)
        {
            break;
        }
    }

    if (map[i].name == NULL)
    {
        return "None";
    }

    return map[i].name;
}

static int intmap_inameb(const char *iname, const struct intmap *map, int *b)
{
    int i;

    for (i = 0; map[i].name != NULL; i++)
    {
        if (!strcasecmp(iname, map[i].name))
        {
            break;
        }
    }

    if (map[i].name == NULL)
    {
        return -1;
    }
    else
    {
        *b = map[i].b;
    }

    return 0;
}

static struct intmap sat_ver_map[] = {{0, AUI_NIM_STD_DVBS2_AUTO, "DVB-S/S2"},
    {1, AUI_NIM_STD_DVBS, "DVB-S"},
    {2, AUI_NIM_STD_DVBS2, "DVB-S2"},
    {0, 0, 0}
};

static struct intmap sat_fec_map[] = {{0, AUI_NIM_FEC_AUTO, "FEC_AUTO"},
    {1, AUI_NIM_FEC_1_2, "FEC_1_2"},
    {2, AUI_NIM_FEC_2_3, "FEC_2_3"},
    {3, AUI_NIM_FEC_3_4, "FEC_3_4"},
    {4, AUI_NIM_FEC_5_6, "FEC_5_6"},
    {5, AUI_NIM_FEC_7_8, "FEC_7_8"},
    {6, AUI_NIM_FEC_1_4, "FEC_1_4"},
    {7, AUI_NIM_FEC_1_3, "FEC 1_3"},
    {8, AUI_NIM_FEC_2_5, "FEC 2_5"},
    {9, AUI_NIM_FEC_3_5, "FEC 3_5"},
    {10, AUI_NIM_FEC_4_5, "FEC 4_5"},
    {11, AUI_NIM_FEC_8_9, "FEC 8_9"},
    {12, AUI_NIM_FEC_9_10, "FEC 9_10"},
    {0, 0, 0}
};

static struct intmap sat_mod_map[] = {{0, AUI_NIM_S_MODUL_QPSK, "QPSK"},
    {1, AUI_NIM_S_MODUL_8PSK, "8PSK"},
    {2, AUI_NIM_S_MODUL_16APSK, "16APSK"},
    {3, AUI_NIM_S_MODUL_32APSK, "32APSK"},
    {0, 0, 0}
};

static struct intmap cab_ver_map[] = {{0, AUI_NIM_STD_DVBC_ANNEX_AC, "DVB-C Annex A/C"},
    {1, AUI_NIM_STD_DVBC_ANNEX_B, "DVB-C Annex B"},
    {0, 0, 0}
};

static struct intmap cab_qam_map[] = {{0, AUI_NIM_NOT_DEFINED, "AUTO-QAM"},
    {1, AUI_NIM_QAM16, "16QAM"},
    {2, AUI_NIM_QAM32, "32QAM"},
    {3, AUI_NIM_QAM64, "64QAM"},
    {4, AUI_NIM_QAM128, "128QAM"},
    {5, AUI_NIM_QAM256, "256QAM"},
    {0, 0, 0}
};

static struct intmap ter_ver_map[] = {{0, AUI_NIM_STD_ISDBT, "ISDB-T"},
    {1, AUI_NIM_STD_DVBT, "DVB-T"},
    {2, AUI_NIM_STD_DVBT2, "DVB-T2"},
    {3, AUI_NIM_STD_DVBT2_COMBO, "DVB-T2 Combo"},
    {0, 0, 0}
};

static struct intmap ter_mod_map[] = {{0, AUI_NIM_MODUL_AUTO, "AUTO-MODULATION"},
    {1, AUI_NIM_MODUL_DQPSK, "DQPSK"},
    {2, AUI_NIM_MODUL_QPSK, "QPSK"},
    {3, AUI_NIM_MODUL_16QAM, "16-QAM"},
    {4, AUI_NIM_MODUL_64QAM, "64-QAM"},
    {5, AUI_NIM_MODUL_256QAM, "256-QAM"},
    {0, 0, 0}
};

static struct intmap ter_fft_map[] = {{0, AUI_NIM_FFT_MODE_AUTO, "AUTO-FFT"},
    {1, AUI_NIM_FFT_MODE_2K, "2K"},
    {2, AUI_NIM_FFT_MODE_4K, "4K"},
    {3, AUI_NIM_FFT_MODE_8K, "8K"},
    {4, AUI_NIM_FFT_MODE_16K, "16K"},
    {5, AUI_NIM_FFT_MODE_32K, "32K"},
    {6, AUI_NIM_FFT_MODE_1K, "1K"},
    {0, 0, 0}
};

static struct intmap ter_gi_map[]             = {{0, AUI_NIM_GUARD_INTER_AUTO, "AUTO-GI"},
    {1, AUI_NIM_GUARD_INTER_1_4, "1/4"},
    {2, AUI_NIM_GUARD_INTER_1_8, "1/8"},
    {3, AUI_NIM_GUARD_INTER_1_16, "1/16"},
    {4, AUI_NIM_GUARD_INTER_1_32, "1/32"},
    {5, AUI_NIM_GUARD_INTER_1_128, "1/128"},
    {6, AUI_NIM_GUARD_INTER_19_256, "19/128"},
    {7, AUI_NIM_GUARD_INTER_19_256, "19/256"},
    {0, 0, 0}
};
static struct intmap ter_hierarchy_type_map[] =
{
    {0, AUI_NIM_DVBT_HIERARCHY_NONE, "NO_HIERARCHY"},
    {1, AUI_NIM_DVBT_HIERARCHY_1, "HIERARCHY_1"},
    {2, AUI_NIM_DVBT_HIERARCHY_2, "HIERARCHY_2"},
    {3, AUI_NIM_DVBT_HIERARCHY_4, "HIERARCHY_4"},
    {4, AUI_NIM_DVBT_HIERARCHY_UNKNOWN, "UNKOWN"},
    {0, 0, 0}
};
static struct intmap ter_profile_type_map[] = {{0, AUI_NIM_DVBT_PROFILE_HP, "PROFILE_HP"},
    {1, AUI_NIM_DVBT_PROFILE_LP, "PROFILE_LP"},
    {2, AUI_NIM_DVBT_PROFILE_UNKNOWN, "UNKOWN"},
    {0, 0, 0}
};
static struct intmap decv_map[]             = {{0, AUI_DECV_FORMAT_MPEG, "MPEG"},
    {1, AUI_DECV_FORMAT_AVC, "AVC"},
    {2, AUI_DECV_FORMAT_AVS, "AVS"},
    {3, AUI_DECV_FORMAT_XVID, "XVID"},
    {4, AUI_DECV_FORMAT_FLV1, "FLV1"},
    {5, AUI_DECV_FORMAT_VP8, "VP8"},
    {6, AUI_DECV_FORMAT_WVC1, "WVC1"},
    {7, AUI_DECV_FORMAT_WX3, "WX3"},
    {8, AUI_DECV_FORMAT_RMVB, "RMVB"},
    {9, AUI_DECV_FORMAT_MJPG, "MJPG"},
    {10, AUI_DECV_FORMAT_HEVC, "HEVC"},
    {11, AUI_DECV_FORMAT_VP9, "VP9"},
    {12, AUI_DECV_FORMAT_H263, "H263"},
    {0, 0, 0}
};

static struct intmap deca_map[] = {{0, AUI_DECA_STREAM_TYPE_MPEG1, "MPEG1"},
    {1, AUI_DECA_STREAM_TYPE_MPEG2, "MPEG2"},
    {2, AUI_DECA_STREAM_TYPE_AAC_LATM, "AAC-LATM"},
    {3, AUI_DECA_STREAM_TYPE_AC3, "AC3"},
    {4, AUI_DECA_STREAM_TYPE_DTS, "DTS"},
    {5, AUI_DECA_STREAM_TYPE_BYE1, "BYE1"},
    {6, AUI_DECA_STREAM_TYPE_RA8, "RA8"},
    {7, AUI_DECA_STREAM_TYPE_MP3, "MP3"},
    {8, AUI_DECA_STREAM_TYPE_AAC_ADTS, "AAC-ADTS"},
    {9, AUI_DECA_STREAM_TYPE_OGG, "OGG"},
    {10, AUI_DECA_STREAM_TYPE_EC3, "EC3"},
    {11, AUI_DECA_STREAM_TYPE_MP3_L3, "MP3-L3"},
    {12, AUI_DECA_STREAM_TYPE_RAW_PCM, "RAW-PCM"},
    {13, AUI_DECA_STREAM_TYPE_BYE1PRO, "BYE1PRO"},
    {14, AUI_DECA_STREAM_TYPE_FLAC, "FLAC"},
    {15, AUI_DECA_STREAM_TYPE_APE, "APE"},
    {16, AUI_DECA_STREAM_TYPE_MP3_2, "MP3-2"},
    {17, AUI_DECA_STREAM_TYPE_AMR, "AMR"},
    {18, AUI_DECA_STREAM_TYPE_ADPCM, "ADPCM"},
    {19, AUI_DECA_STREAM_TYPE_OPUS, "OPUS"},
    {21, AUI_DECA_STREAM_TYPE_VORBIS, "VORBIS"},
    {0, 0, 0}
};

static struct intmap chg_mode_map[] = {{0, AUI_DECV_CHG_BLACK, "BLACK"},
    {1, AUI_DECV_CHG_STILL, "STILL"},
    {2, AUI_DECV_CHG_OTV_PFM, "PFM"},
    {0, 0, 0}
};

aui_nim_demod_type _to_aui_nim_demod_type(const char *input)
{
    aui_nim_demod_type type = AUI_NIM_QPSK;
    int num                 = 0;

    if (!strncasecmp(input, "S", 1))
    {
        type = AUI_NIM_QPSK;
    }
    else if (!strncasecmp(input, "C", 1))
    {
        type = AUI_NIM_QAM;
    }
    else if (!strncasecmp(input, "T", 1))
    {
        type = AUI_NIM_OFDM;
    }
    else if (!strncasecmp(input, "D", 1))
    {
        type = AUI_NIM_QPSK;
    }
    else if (!strncasecmp(input, "D188", 4))
    {
        type = AUI_NIM_QPSK;
    }
    else
    {
        num = atoi(input);

        if (num <= AUI_NIM_OFDM)
        {
            type = (aui_nim_demod_type)num;
        }
        else
        {
            type = AUI_NIM_QPSK;
        }
    }

    return type;
}

aui_tsi_transport_type _to_aui_transport_type(const char *input)
{
    aui_tsi_transport_type type = AUI_TSI_TRANSPORT_TYPE_TS;
    int num                     = 0;

    if (!strncasecmp(input, "D188", 4))
    {
        type = AUI_TSI_TRANSPORT_TYPE_DSS_ALI_188_ENCAPSULATED;
    }
    else if (!strncasecmp(input, "D", 1))
    {
        type = AUI_TSI_TRANSPORT_TYPE_DSS;
    }
    else if (!strncasecmp(input, "S", 1))
    {
        type = AUI_TSI_TRANSPORT_TYPE_TS;
    }
    else if (!strncasecmp(input, "C", 1))
    {
        type = AUI_TSI_TRANSPORT_TYPE_TS;
    }
    else if (!strncasecmp(input, "T", 1))
    {
        type = AUI_TSI_TRANSPORT_TYPE_TS;
    }
    else
    {
        num = atoi(input);

        if (num <= AUI_NIM_OFDM)
        {
            type = AUI_TSI_TRANSPORT_TYPE_TS;
        }
        else
        {
            /*if the input parameter > 2
                       for ali inter test, means DSS stream formated to 188
                       for customer, means DSS stream, type should be set to
               AUI_TSI_TRANSPORT_TYPE_DSS
                        */
            type = AUI_TSI_TRANSPORT_TYPE_DSS_ALI_188_ENCAPSULATED;
        }
    }

    return type;
}

aui_nim_std _to_aui_nim_std(const char *input)
{
    aui_nim_std std = AUI_NIM_STD_DVBC_ANNEX_AC;

    if (!strcasecmp(input, "AC"))
    {
        std = AUI_NIM_STD_DVBC_ANNEX_AC;
    }
    else if (!strcasecmp(input, "B"))
    {
        std = AUI_NIM_STD_DVBC_ANNEX_B;
    }
    else if (!strcasecmp(input, "ISDB-T"))
    {
        std = AUI_NIM_STD_ISDBT;
    }
    else if (!strcasecmp(input, "T"))
    {
        std = AUI_NIM_STD_DVBT;
    }
    else if (!strcasecmp(input, "T2"))
    {
        std = AUI_NIM_STD_DVBT2;
    }
    else if (!strcasecmp(input, "T2-COMBO"))
    {
        std = AUI_NIM_STD_DVBT2_COMBO;
    }
    else if (!strcasecmp(input, "AUTO"))
    {
        std = AUI_NIM_STD_DVBS2_AUTO;
    }
    else if (!strcasecmp(input, "S"))
    {
        std = AUI_NIM_STD_DVBS;
    }
    else if (!strcasecmp(input, "S2"))
    {
        std = AUI_NIM_STD_DVBS2;
    }
    else
    {
        std = atoi(input);
    }

    return std;
}

aui_nim_polar _to_aui_nim_polar(const char *input)
{
    aui_nim_polar polar = AUI_NIM_POLAR_HORIZONTAL;

    if (!strncasecmp("H", input, 1))
    {
        polar = AUI_NIM_POLAR_HORIZONTAL;
    }
    else if (!strncasecmp("V", input, 1))
    {
        polar = AUI_NIM_POLAR_VERTICAL;
    }
    else if (!strncasecmp("L", input, 1))
    {
        polar = AUI_NIM_CPOLAR_LEFT;
    }
    else if (!strncasecmp("R", input, 1))
    {
        polar = AUI_NIM_CPOLAR_RIGHT;
    }
    else
    {
        polar = atoi(input);
    }

    return polar;
}

aui_nim_qam_mode _to_aui_nim_qam_mode(const char *input)
{
    aui_nim_qam_mode qam = AUI_NIM_NOT_DEFINED;
    qam                  = atoi(input);

    if ((int)qam > 15)
    {
        switch ((int)qam)
        {
            case 16:
                qam = AUI_NIM_QAM16;
                break;

            case 32:
                qam = AUI_NIM_QAM32;
                break;

            case 64:
                qam = AUI_NIM_QAM64;
                break;

            case 128:
                qam = AUI_NIM_QAM128;
                break;

            case 256:
                qam = AUI_NIM_QAM256;
                break;

            default:
                qam = AUI_NIM_NOT_DEFINED;
                break;
        }
    }

    return qam;
}

aui_nim_bandwidth _to_aui_nim_bandwidth(const char *input)
{
    aui_nim_bandwidth band = AUI_NIM_BANDWIDTH_AUTO;
    band                   = atoi(input);

    switch ((int)band)
    {
        case 1:
            band = AUI_NIM_BANDWIDTH_1_7_MHZ;
            break;

        case 5:
            band = AUI_NIM_BANDWIDTH_5_MHZ;
            break;

        case 6:
            band = AUI_NIM_BANDWIDTH_6_MHZ;
            break;

        case 7:
            band = AUI_NIM_BANDWIDTH_7_MHZ;
            break;

        case 8:
            band = AUI_NIM_BANDWIDTH_8_MHZ;
            break;

        case 10:
            band = AUI_NIM_BANDWIDTH_10_MHZ;
            break;

        default:
            printf("wrong bandwidth %s\n", input);
    }

    return band;
}

aui_decv_format _to_aui_decv(const char *input)
{
    aui_decv_format decv = AUI_DECV_FORMAT_MPEG;

    if (intmap_inameb(input, decv_map, (int *)&decv))
    {
        decv = atoi(input);
    }

    return decv;
}

aui_deca_stream_type _to_aui_deca(const char *input)
{
    aui_deca_stream_type deca = AUI_DECA_STREAM_TYPE_MPEG1;

    if (intmap_inameb(input, deca_map, (int *)&deca))
    {
        deca = atoi(input);
    }

    return deca;
}

aui_decv_chg_mode _to_aui_decv_chg_mode(const char *input)
{
    aui_decv_chg_mode chg = AUI_DECV_CHG_BLACK;

    if (intmap_inameb(input, chg_mode_map, (int *)&chg))
    {
        chg = atoi(input);
    }

    return chg;
}

aui_nim_profile_type _to_aui_nim_profile_type(const char *input)
{
    aui_nim_profile_type priority = AUI_NIM_DVBT_PROFILE_HP;

    if (!strcasecmp(input, "HP"))
    {
        priority = AUI_NIM_DVBT_PROFILE_HP;
    }
    else if (!strcasecmp(input, "LP"))
    {
        priority = AUI_NIM_DVBT_PROFILE_LP;
    }
    else
    {
        priority = atoi(input);
    }

    return priority;
}

unsigned short g_stream_pids[4] =
{
    513 /* video */, 660 /* audio */, 8190 /* pcr */, 8190 /*ad pid */
};
unsigned long s_play_mode = 0;
int nim_get_signal_info(struct tpm_nim_init_para *para);

int g_nim_connect_ret  = 0; // The resule of connect operation
int g_nim_connect_time = 0; // The time which aui_nim_connect takes
int g_nim_begint_time  = 0; // Record the time of aui_nim_connect

struct connect_data
{
    aui_hdl hdl;
    struct aui_nim_connect_param param;
};
struct connect_data g_c_data;

static void nim_connecting(void *p)
{
    struct connect_data *data = (struct connect_data *)p;
    AUI_RTN_CODE ret          = 0;
    prctl(PR_SET_NAME, __func__);
    g_nim_begint_time  = osal_get_tick();
    ret                = aui_nim_connect(data->hdl, &data->param);
    g_nim_connect_time = osal_get_tick() - g_nim_begint_time;

    if (ret != AUI_RTN_SUCCESS)
    {
        g_nim_connect_ret = -1;
        printf("error: aui_nim_connect\n");
    }
    else
    {
        g_nim_connect_ret = 0;
        printf("NIM aui_nim_connect done\n");

        if (data->param.en_demod_type == AUI_NIM_OFDM)
        {
            printf("After connect DVB-T std: %d\n", data->param.connect_param.ter.std);
        }
    }
}

int connect_task(aui_hdl hdl, struct aui_nim_connect_param *param)
{
    memset(&g_c_data, 0, sizeof(g_c_data));
    g_c_data.param = *param;
    g_c_data.hdl   = hdl;
#ifdef AUI_TDS
    OSAL_ID connect_thread = OSAL_INVALID_ID;
    T_CTSK task_param;
    task_param.task    = nim_connecting;
    task_param.name[0] = 'L';
    task_param.name[1] = 'C';
    task_param.name[2] = 'K';
    task_param.quantum = 10;
    task_param.itskpri = OSAL_PRI_NORMAL;
    task_param.stksz   = 0xd000 * 8;
    task_param.para1   = (DWORD)&g_c_data;
    task_param.para2   = 0;
    connect_thread = osal_task_create(&task_param);

    if (connect_thread == OSAL_INVALID_ID)
    {
        printf("Fail create connect task fail\n");
        return -1;
    }
    else
    {
        printf("Run task %s in task %d\n", "LCK", connect_thread);
    }

#else // #ifdef AUI_TDS
    pthread_t connect_thread;
    pthread_attr_t thread_attr;
    int pthread_create_ret = 0;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    pthread_create_ret = pthread_create(
                             &connect_thread, &thread_attr, (void *)&nim_connecting, (void *)&g_c_data);

    if (pthread_create_ret)
    {
        printf("Fail create connect thread\n");
        return -1;
    }

#ifdef AUI_ANDROID
    signal(SIGQUIT, nim_signal_handler);
#endif
#endif // #ifdef AUI_TDS
    return 0;
}

int nim_connect(struct tpm_nim_init_para *para, aui_hdl *aui_nim_hdl)
{
#ifndef CONFIG_ALI_EMULATOR
    aui_hdl hdl = NULL;
    struct aui_nim_connect_param param;
    int as_high_band        = AUI_NIM_LOW_BAND;
    unsigned long temp_freq = 0;
    int ret                 = 0;
    g_nim_begint_time       = 0;
    g_nim_connect_time      = 0;
    g_nim_connect_ret       = 0;
    /*init nim_attr and param variable*/
    MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));

    if (aui_nim_handle_get_byid(para->ul_device, &hdl))
    {
        // printf("%d aui_nim_handle_get_byid: NIM %d is not open yet\n",
        //        __LINE__, para->ul_device);
    }

    if (hdl != NULL && para->ul_nim_type == (unsigned long)AUI_NIM_QAM)
    {
        if ((unsigned long)g_nims_std[para->ul_device] != para->ul_nim_std)
        {
            printf("[DVB-C] NIM std changed, re-open it.\n");

            if (aui_nim_close(hdl) != AUI_RTN_SUCCESS)
            {
                printf("DVB-C aui_nim_close fail\n");
                return -1;
            }

            hdl = NULL;
        }
        else
        {
            printf("[DVB-C] NIM %lu std %lu.\n", para->ul_device, para->ul_nim_std);
        }
    }
    else
    {
        // printf("Not a [DVB-C] NIM\n");
    }

    if (hdl == NULL)
    {
        struct aui_nim_attr nim_attr;
        MEMSET(&nim_attr, 0, sizeof(struct aui_nim_attr));
        nim_attr.ul_nim_id    = para->ul_device;
        nim_attr.en_dmod_type = para->ul_nim_type;
        nim_attr.en_std       = para->ul_nim_std; // added by vedic.fu

        if (nim_attr.en_dmod_type == AUI_NIM_OFDM)
        {
            if (nim_attr.en_std != AUI_NIM_STD_ISDBT)
            {
                nim_attr.en_std = AUI_NIM_STD_DVBT2_COMBO;
            }
        }

        printf(
            "\nul_nim_std = %lu, ul_nim_id = %lu\n", nim_attr.en_std, nim_attr.ul_nim_id);

        if (aui_nim_open(&nim_attr, &hdl))
        {
            printf("open error\n");
            return -1;
        }

        g_nims_std[nim_attr.ul_nim_id] = nim_attr.en_std;
        printf("Open NIM %d OK\n", (int)nim_attr.ul_nim_id);
    }

    switch (para->ul_nim_type)   /*signal modulation types:DVB-S,DVB-C,DVB-T*/
    {
        case AUI_NIM_QPSK:           /*DVB-S*/
            // All C-band LNB have a local oscillator (L.O.) frequency of 5.150 GHz
            // but Ku-band LNB may come in many different frequencies
            // typically 9.750 to 12.75 GHz.
            // C-band dual polarity LNB (One-Cable-Solution)
            // Different polarizations use different  L.O Frequency, So users can receive
            // different polarizations signal at same time.
            // Polarization  Frequency band  L.O Frequency  I.F.
            // Horizontal    3.7-4.2 GHz     5.15 GHz       950-1,450 MHz
            // Vertical      3.7-4.2 GHz     5.75 GHz       1,550-2,050 MHz
            // Ku-band LNB
            // Europe Universal LNB ("Astra" LNB)
            // Voltage  Tone    Polarization  Frequency        band  L.O Frequency  I.F.
            // 13 V     0 kHz   Vertical      10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
            // 18 V     0 kHz   Horizontal    10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
            // 13 V     22 kHz  Vertical      11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz
            // 18 V     22 kHz  Horizontal    11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz
            temp_freq = para->ul_freq;
            (void)temp_freq;

            if (para->lnbftype == 1)
            {
                param.ul_freq = para->ul_polar == AUI_NIM_POLAR_HORIZONTAL ? para->ul_freq - LO_11400 : para->ul_freq - LO_10250;
            }
            else
            {
                if (para->ul_freq > LO_9750)
                {
                    if (para->ul_freq > KU_11700)
                    {
                        as_high_band  = AUI_NIM_HIGH_BAND;
                        param.ul_freq = para->ul_freq - LO_10600;
                    }
                    else
                    {
                        as_high_band  = AUI_NIM_LOW_BAND;
                        param.ul_freq = para->ul_freq - LO_9750;
                    }
                }
                else
                {
                    as_high_band = AUI_NIM_LOW_BAND;

                    if (para->ul_polar)
                    {
                        param.ul_freq = LO_5750 - para->ul_freq;
                    }
                    else
                    {
                        param.ul_freq = LO_5150 - para->ul_freq;
                    }
                }
            }

            param.en_demod_type                       = AUI_NIM_QPSK;
            param.connect_param.sat.ul_freq_band      = as_high_band;
            param.connect_param.sat.ul_symrate        = para->ul_symb_rate;
            param.connect_param.sat.fec               = AUI_NIM_FEC_AUTO;
            param.connect_param.sat.ul_polar          = para->ul_polar;
            param.connect_param.sat.ul_src            = para->ul_src;
            param.connect_param.sat.ul_plsn           = para->ul_plsn_value;
            param.connect_param.sat.ul_stream_id      = para->stream_id;
            param.connect_param.sat.ul_tsn            = para->tsn;
            param.connect_param.sat.uc_plhs           = para->plhs;
            param.connect_param.sat.ul_plhs_seq_31_0  = para->plhs_seq_31_0;
            param.connect_param.sat.ul_plhs_seq_63_32 = para->plhs_seq_63_32;
            param.connect_param.sat.ul_plhs_seq_89_64 = para->plhs_seq_89_64;
            param.connect_param.sat.std               = para->ul_nim_std;
            printf("[%s]%d freq:%lu (I.F. %u), sym:%lu, polar:%u, band:%u, src:%lu,\n"
                   "plsn: %lu, tsn:%lu, stream_id:%lu\n"
                   "(DirecTV)plhs:%s, 89_64:%08X, 63_32:%08X, 31_0:%08X\n\n",
                   intmap_bname(param.connect_param.sat.std, sat_ver_map),
                   param.connect_param.sat.std,
                   temp_freq,
                   param.ul_freq,
                   param.connect_param.sat.ul_symrate,
                   param.connect_param.sat.ul_polar,
                   param.connect_param.sat.ul_freq_band,
                   param.connect_param.sat.ul_src,
                   param.connect_param.sat.ul_plsn,
                   param.connect_param.sat.ul_tsn,
                   param.connect_param.sat.ul_stream_id,
                   param.connect_param.sat.uc_plhs ? "Enable" : "Disable",
                   param.connect_param.sat.ul_plhs_seq_89_64,
                   param.connect_param.sat.ul_plhs_seq_63_32,
                   param.connect_param.sat.ul_plhs_seq_31_0);
            break;

        case AUI_NIM_QAM: /*DVB-C*/
            param.ul_freq                       = para->ul_freq;
            param.en_demod_type                 = AUI_NIM_QAM;
            param.connect_param.cab.ul_symrate  = para->ul_symb_rate;
            param.connect_param.cab.en_qam_mode = para->qam_mode;
            param.connect_param.cab.bandwidth   = para->ul_freq_band;
            printf("Connecting DVB-C freq:%lu , sym:%lu, qam_mode: %lu, "
                   "band width: %lu\n",
                   param.ul_freq,
                   param.connect_param.sat.ul_symrate,
                   param.connect_param.cab.en_qam_mode,
                   param.connect_param.cab.bandwidth);
            break;

        case AUI_NIM_OFDM: /*DVB-T*/
            param.ul_freq                     = para->ul_freq;
            param.en_demod_type               = AUI_NIM_OFDM;
            param.connect_param.ter.bandwidth = para->ul_freq_band;
            param.connect_param.ter.std       = para->ul_nim_std;
            param.connect_param.ter.fec       = AUI_NIM_FEC_AUTO;
            param.connect_param.ter.spectrum  = AUI_NIM_SPEC_AUTO;

            if (param.connect_param.ter.std == AUI_NIM_STD_DVBT)
            {
                param.connect_param.ter.u.dvbt.priority = para->priority;
            }
            else
            {
                param.connect_param.ter.u.dvbt2.plp_id    = para->plp_id;
                param.connect_param.ter.u.dvbt2.plp_index = para->plp_index;
            }

            printf("[NIM CONNECT] freq: %d demod: %d bandwidth: %d fec: %d "
                   "fft: %d guard: %d modulation: %d spectrum: %d std: %d \n",
                   param.ul_freq,
                   param.en_demod_type,
                   param.connect_param.ter.bandwidth,
                   param.connect_param.ter.fec,
                   param.connect_param.ter.fft_mode,
                   param.connect_param.ter.guard_interval,
                   param.connect_param.ter.modulation,
                   param.connect_param.ter.spectrum,
                   param.connect_param.ter.std);

            if (param.connect_param.ter.std == AUI_NIM_STD_DVBT)
            {
                printf("DVB-T priority: %s\n",
                       param.connect_param.ter.u.dvbt.priority == AUI_NIM_DVBT_PROFILE_LP
                       ? "Low"
                       : "HIGH");
            }
            else
            {
                printf("DVB-T2 plp_id: %d plp_index: %d\n",
                       param.connect_param.ter.u.dvbt2.plp_id,
                       param.connect_param.ter.u.dvbt2.plp_index);
            }

            break;

        default:
            printf("not supported NIM type %d\n", (int)para->ul_nim_type);
            return -1;
    }

    if (para->ul_nim_type != AUI_NIM_QPSK)
    {
        // aui_nim_connect of DVB-C/T may take 4s in some cases,
        // so put it in another thread.
        ret = connect_task(hdl, &param);
    }
    else
    {
        g_nim_begint_time  = osal_get_tick();
        ret                = aui_nim_connect(hdl, &param);
        g_nim_connect_time = osal_get_tick() - g_nim_begint_time;
    }

    if (ret != 0)
    {
        return ret;
    }

    *aui_nim_hdl = hdl;
#endif // CONFIG_ALI_EMULATOR
    return 0;
}

int nim_get_signal_info(struct tpm_nim_init_para *para)
{
    aui_hdl hdl         = 0;
    unsigned int dev_id = para->ul_device;
    int lock_time       = 0;
    (void)lock_time;
    int time_a       = 0;
    int lock_status  = AUI_NIM_STATUS_UNKNOWN;
    int as_high_band = para->ul_freq > KU_11700 ? AUI_NIM_HIGH_BAND : AUI_NIM_LOW_BAND;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (aui_nim_handle_get_byid(dev_id, &hdl))
    {
        printf("%d %s NIM %d not found\n", __LINE__, __FUNCTION__, dev_id);
        return -1;
    }

    // Driver need at lease 50MS to prepare the signal info after locked
    AUI_SLEEP(100); // wait 100MS
    // Wait NIM locked in NIM_TIMEOUT_MS
    printf("\nWait NIM locked in %d MS ...\n\n", NIM_TIMEOUT_MS);
    lock_status = AUI_NIM_STATUS_UNKNOWN;

    while (1)
    {
        time_a = osal_get_tick();
        ret    = aui_nim_is_lock(hdl, &lock_status);
        time_a = osal_get_tick() - time_a;

        if (time_a > 300)
        {
            printf("\n");
            printf("[Warning]aui_nim_is_lock take too much time: %d MS\n", time_a);
            printf("\n");
        }
        else
        {
            // printf("aui_nim_is_lock take time: %d MS\n", time_a);
        }

        if (ret != AUI_RTN_SUCCESS)
        {
            printf("%d aui_nim_is_lock error\n", __LINE__);
            return -1;
        }
        else
        {
            if (lock_status == AUI_NIM_STATUS_LOCKED)
            {
                lock_time = (int)(osal_get_tick() - g_nim_begint_time);
                break;
            }
        }

        AUI_SLEEP(10);

        if ((osal_get_tick() - g_nim_begint_time) >= NIM_TIMEOUT_MS)
        {
            break;
        }

        if (g_nim_connect_ret)
        {
            return -1;
        }
    }

    aui_signal_status info;
    memset(&info, 0, sizeof(info));

    if (aui_nim_signal_info_get(hdl, &info))
    {
        printf("%d info_get error\n", __LINE__);
    }
    else
    {
        printf("IF: %lu Strength: %lu%%, Quality: %lu%%, "
               "Level: %.1f dBm(%ld)\n",
               info.ul_freq,
               info.ul_signal_strength,
               info.ul_signal_quality,
               info.ul_rf_level * (-0.1),
               info.ul_rf_level);
    }

    printf("\n------------------------------\n");
    printf("[NIM %ld]: aui_nim_connect takes %d MS\n", para->ul_device, g_nim_connect_time);

    if (lock_status != AUI_NIM_STATUS_LOCKED)
    {
        printf("\n>>>> [Warning]Signal is not locked in %d MS\n", NIM_TIMEOUT_MS);
        printf("------------------------------\n\n");
        return -1;
    }
    else
    {
        printf("[NIM %ld]: Locking takes %d MS\n", para->ul_device, lock_time);
    }

    printf("------------------------------\n\n");
    // Get signal info several time after locked
    int tmp_cnt = 10; //

    while (tmp_cnt-- > 0)
    {
        if (aui_nim_signal_info_get(hdl, &info))   /*getting the infomation of current tp*/
        {
            printf("%d info_get error\n", __LINE__);
            return -1;
        }

        double temp_freq = 0;
        double mhz       = 0;
        temp_freq        = info.ul_freq;
        mhz              = (double)info.ul_freq;

        if (para->ul_nim_type == AUI_NIM_QPSK)   /*DVB-S*/
        {
            if (para->lnbftype == 1)
            {
                temp_freq += para->ul_polar == AUI_NIM_POLAR_HORIZONTAL ? LO_11400 : LO_10250;
            }
            else
            {
                if (para->ul_freq > LO_9750)
                {
                    if (as_high_band == AUI_NIM_HIGH_BAND)
                    {
                        temp_freq += LO_10600;
                    }
                    else
                    {
                        temp_freq += LO_9750;
                    }
                }
                else
                {
                    if (para->ul_polar)
                    {
                        temp_freq = LO_5750 - info.ul_freq;
                    }
                    else
                    {
                        temp_freq = LO_5150 - info.ul_freq;
                    }
                }
            }

            printf("[NIM %ld] std: %s(%d), %s(%d), %s(%d), %ldKbd\n",
                   para->ul_device,
                   intmap_bname(info.std, sat_ver_map),
                   info.std,
                   intmap_bname(info.u.dvbs2.mudulation, sat_mod_map),
                   info.u.dvbs2.mudulation,
                   intmap_bname(info.u.dvbs2.fec, sat_fec_map),
                   info.u.dvbs2.fec,
                   info.u.dvbs2.sym);
        }
        else if (para->ul_nim_type == AUI_NIM_QAM)
        {
            mhz /= 100.0;
            temp_freq /= 100.0;
            printf("[NIM  %ld] std: %s(%d) %ldKbd %s(%d)\n",
                   para->ul_device,
                   intmap_bname(info.std, cab_ver_map),
                   info.std,
                   info.u.dvbc.sym,
                   intmap_bname(info.u.dvbc.qam_mode, cab_qam_map),
                   info.u.dvbc.qam_mode);
        }
        else if (para->ul_nim_type == AUI_NIM_OFDM)
        {
            mhz /= 1000.0;
            temp_freq /= 1000.0;
            printf("[NIM  %ld] std: %s(%d)\n",
                   para->ul_device,
                   intmap_bname(info.std, ter_ver_map),
                   info.std);
        }

        printf("IF: %.3f MHz[%.3f MHz](%lu) Strength: %lu%%, Quality: %lu%%, "
               "Level: %.1f dBm(%lu)\n"
               "Pre-BER: %.2e(%lu), BER: %.2e(%lu), "
               "CNR: %.2f dB(%lu), MER: %.1f dB(%lu), "
               "PER: %.2e(%lu)\n",
               mhz,
               temp_freq,
               info.ul_freq,
               info.ul_signal_strength,
               info.ul_signal_quality,
               info.ul_rf_level * (-0.1),
               info.ul_rf_level,
               info.ul_pre_ber * (1.0E-7),
               info.ul_pre_ber,
               info.ul_bit_error_rate * (1.0E-7),
               info.ul_bit_error_rate,
               info.ul_signal_cn * 0.01,
               info.ul_signal_cn,
               info.ul_mer * 0.1,
               info.ul_mer,
               info.ul_per * (1.0E-5),
               info.ul_per);

        if (para->ul_nim_type == AUI_NIM_OFDM)
        {
            if (info.std == AUI_NIM_STD_ISDBT)
            {
                printf("EWBS: %lu\n", info.u.dvbt_isdbt.ul_ewbs_flag);
                printf("Layer A Segment Count: %lu, BER: %.2e(%lu)\n"
                       "Layer B Segment Count: %lu, BER: %.2e(%lu)\n"
                       "Layer C Segment Count: %lu, BER: %.2e(%lu)\n",
                       info.u.dvbt_isdbt.ul_isdbt_layera_seg_number,
                       info.u.dvbt_isdbt.ul_isdbt_layera_ber * (1.0E-7),
                       info.u.dvbt_isdbt.ul_isdbt_layera_ber,
                       info.u.dvbt_isdbt.ul_isdbt_layerb_seg_number,
                       info.u.dvbt_isdbt.ul_isdbt_layerb_ber * (1.0E-7),
                       info.u.dvbt_isdbt.ul_isdbt_layerb_ber,
                       info.u.dvbt_isdbt.ul_isdbt_layerc_seg_number,
                       info.u.dvbt_isdbt.ul_isdbt_layerc_ber * (1.0E-7),
                       info.u.dvbt_isdbt.ul_isdbt_layerc_ber);
            }
            else if (info.std == AUI_NIM_STD_DVBT2)
            {
                printf(
                    "plp_num: %d plp_id: %d\n", info.u.dvbt2.plp_num, info.u.dvbt2.plp_id);
                printf("%s(%d) FFT=%s(%d) G=%s(%d) %s(%d)\n",
                       intmap_bname(info.u.dvbt2.fec, sat_fec_map),
                       info.u.dvbt2.fec,
                       intmap_bname(info.u.dvbt2.fft_mode, ter_fft_map),
                       info.u.dvbt2.fft_mode,
                       intmap_bname(info.u.dvbt2.guard_inter, ter_gi_map),
                       info.u.dvbt2.guard_inter,
                       intmap_bname(info.u.dvbt2.modulation, ter_mod_map),
                       info.u.dvbt2.modulation);
                printf("siso_simo=%d cell_id=%d network_id=%d system_id=%d\n",
                       info.u.dvbt2.siso_miso,
                       info.u.dvbt2.cell_id,
                       info.u.dvbt2.network_id,
                       info.u.dvbt2.system_id);
            }
            else if (info.std == AUI_NIM_STD_DVBT)
            {
                printf("%s(%d) HP_CODE=%s(%d) FFT=%s(%d) G=%s(%d) %s(%d)\n",
                       intmap_bname(info.u.dvbt_isdbt.fec, sat_fec_map),
                       info.u.dvbt_isdbt.fec,
                       intmap_bname(info.u.dvbt_isdbt.hp_fec, sat_fec_map),
                       info.u.dvbt_isdbt.hp_fec,
                       intmap_bname(info.u.dvbt_isdbt.fft_mode, ter_fft_map),
                       info.u.dvbt_isdbt.fft_mode,
                       intmap_bname(info.u.dvbt_isdbt.guard_inter, ter_gi_map),
                       info.u.dvbt_isdbt.guard_inter,
                       intmap_bname(info.u.dvbt_isdbt.modulation, ter_mod_map),
                       info.u.dvbt_isdbt.modulation);
                printf(
                    "hierarchy_type=%s(%d) profile_type=%s(%d)\n",
                    intmap_bname(info.u.dvbt_isdbt.hierarchy_type, ter_hierarchy_type_map),
                    info.u.dvbt_isdbt.hierarchy_type,
                    intmap_bname(info.u.dvbt_isdbt.profile_type, ter_profile_type_map),
                    info.u.dvbt_isdbt.profile_type);
            }
        }

        printf("\n");
        AUI_SLEEP(200); // MS
    }

    return 0;
}

static enum aui_tsi_output_id hwdmx_id_to_tsi_output_id(enum aui_dmx_id hwdmx_id)
{
    enum aui_tsi_output_id tsi_output_id = -1;

    switch (hwdmx_id)
    {
        case AUI_DMX_ID_DEMUX0:
        {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_0;
            break;
        }

        case AUI_DMX_ID_DEMUX1:
        {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_1;
            break;
        }

        case AUI_DMX_ID_DEMUX2:
        {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_2;
            break;
        }

        case AUI_DMX_ID_DEMUX3:
        {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_3;
            break;
        }

        default:
        {
            printf("Not support hwdmx_id [%d]\n", hwdmx_id);
            break;
        }
    }

    return tsi_output_id;
}

#if 0
int tpm_tsi_init(struct tpm_tsi_init_para *para, aui_hdl *p_hdl)
{
    if (aui_find_dev_by_idx(AUI_MODULE_TSI, 0, p_hdl))
    {
        if (aui_tsi_open(p_hdl))
        {
            printf("\r\n aui_tsi_open error \n");
            return -1;
        }
    }

    aui_tsi_input_id input_id = 0xffff;

    if (tsi_original_input_id_get(*p_hdl, para->ul_nim_index, &input_id))
    {
        printf("%s %d: tsi_original_input_id_get fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // from TSG, the tsi input id is fixed to AUI_TSI_INPUT_TSG
    if (AUI_NIM_HANDLER_MAX == para->ul_nim_index)
    {
        input_id = AUI_TSI_INPUT_TSG;
        printf("tsg play, set tsi input id to AUI_TSI_INPUT_TSG.\n");
    }

    enum aui_tsi_channel_id channel_id = para->ul_tis_port_idx;
    // for compatible hwdmx2 and hwdmx3
    enum aui_tsi_output_id tsi_output_id = hwdmx_id_to_tsi_output_id(para->ul_dmx_idx);
    printf("%s %d: tsi input_id=%d, channel_id=%d, output_id=%d\n",
           __FUNCTION__,
           __LINE__,
           input_id,
           channel_id,
           tsi_output_id);

    if (aui_tsi_route_cfg(*p_hdl, input_id, channel_id, tsi_output_id))
    {
        printf("\r\n aui_tsi_route_cfg error \n");
        return -1;
    }

    enum aui_tsi_transport_type transport_type = para->ul_transport_type;

    /*
         set transport type, TS stream or Directv stream
        */
    if (aui_tsi_transport_type_set(*p_hdl, transport_type, channel_id))
    {
        printf("\r\n aui_tsi_transport_type_set error \n");
        return -1;
    }

    printf("\r\n aui_tsi_transport_type_set %d.\n", transport_type);
    return 0;
}
#endif

#if 0
int tpm_tsi_init_cic(struct tpm_tsi_init_para *para, aui_hdl *p_hdl)
{
    if (aui_find_dev_by_idx(AUI_MODULE_TSI, 0, p_hdl))
    {
        if (aui_tsi_open(p_hdl))
        {
            printf("\r\n aui_tsi_open error \n");
            return -1;
        }
    }

#if 0
    // TASK #44551, do not call aui_tsi_CIConnect_mode_set here,
    // the aui_tsi_ci_card_bypass_set would set the cic connect mode.
    printf("Enter aui_tsi_CIConnect_mode_set\n");

    if (aui_tsi_CIConnect_mode_set(*p_hdl, 1))
    {
        printf("aui_tsi_CIConnect_mode_set fail\n");
    }

#endif
    aui_tsi_input_id input_id = 0xffff;

    if (tsi_original_input_id_get(*p_hdl, para->ul_nim_index, &input_id))
    {
        printf("%s %d: tsi_original_input_id_get fail\n", __func__, __LINE__);
        return -1;
    }

    // from TSG, the tsi input id is fixed to AUI_TSI_INPUT_TSG
    if (AUI_NIM_HANDLER_MAX == para->ul_nim_index)
    {
        input_id = AUI_TSI_INPUT_TSG;
        printf("tsg play, set tsi input id to AUI_TSI_INPUT_TSG.\n");
    }

    enum aui_tsi_channel_id channel_id = para->ul_tis_port_idx;
    // for compatible hwdmx2 and hwdmx3
    enum aui_tsi_output_id tsi_output_id = hwdmx_id_to_tsi_output_id(para->ul_dmx_idx);
    printf("%s %d: tsi input_id=%d, channel_id=%d, output_id=%d\n",
           __FUNCTION__,
           __LINE__,
           input_id,
           channel_id,
           tsi_output_id);

    if (aui_tsi_route_cfg(*p_hdl, input_id, channel_id, tsi_output_id))
    {
        printf("\r\n aui_tsi_route_cfg error \n");
        return -1;
    }

    printf("Enter aui_tsi_ci_card_bypass_set\n");

    if (aui_tsi_ci_card_bypass_set(*p_hdl, 0, 0))   // pass CI card
    {
        printf("aui_tsi_ci_card_bypass_set fail\n");
    }

    printf("aui_tsi_ci_card_bypass_set pass CI card success\n");
    return 0;
}
#endif

int set_dmx_for_create_av_stream(int dmx_id,
                                 unsigned short video_pid,
                                 unsigned short audio_pid,
                                 unsigned short audio_desc_pid,
                                 unsigned short pcr_pid,
                                 aui_hdl *dmx_hdl)
{
    aui_hdl hdl = NULL;
    aui_attr_dmx attr_dmx;
    aui_dmx_stream_pid pid_list;
    MEMSET(&attr_dmx, 0, sizeof(aui_attr_dmx));
    MEMSET(&pid_list, 0, sizeof(aui_dmx_stream_pid));
    attr_dmx.uc_dev_idx = dmx_id;

    if (aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx, &hdl))
    {
        if (aui_dmx_open(&attr_dmx, &hdl))
        {
            printf("\r\n dmx open fail\n");
            return -1;
        }
    }

    if (NULL == hdl)
    {
        return -1;
    }

    if (aui_dmx_start(hdl, &attr_dmx))
    {
        aui_dmx_stop(hdl, NULL);             /*starting fail may not be close dmx*/

        if (aui_dmx_start(hdl, &attr_dmx))   /*start again*/
        {
            printf("\r\n aui_dmx_start fail\n");
            aui_dmx_close(hdl);
            return AUI_RTN_FAIL;
        }
    }

    pid_list.ul_pid_cnt      = 4;
    pid_list.stream_types[0] = AUI_DMX_STREAM_VIDEO;
    pid_list.stream_types[1] = AUI_DMX_STREAM_AUDIO;
    pid_list.stream_types[2] = AUI_DMX_STREAM_PCR;
    pid_list.stream_types[3] = AUI_DMX_STREAM_AUDIO_DESC;
    pid_list.aus_pids_val[0] = video_pid;
    pid_list.aus_pids_val[1] = audio_pid;
    pid_list.aus_pids_val[2] = pcr_pid;
    pid_list.aus_pids_val[3] = audio_desc_pid;
    printf("\r\nhdl:video_pid,audio_pid,pcr_pid,audio_desc_pid=[%08x][%d][%d][%d][%d]",
           (int)hdl,
           video_pid,
           audio_pid,
           pcr_pid,
           audio_desc_pid);

    if (0 == s_play_mode)
    {
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list))
        {
            printf("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
    }
    else if (1 == s_play_mode)
    {
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AUDIO, &pid_list))
        {
            printf("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AUDIO fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
    }
    else
    {
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_VIDEO, &pid_list))
        {
            printf("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_VIDEO fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
    }

    *dmx_hdl = hdl;
    return 0;
}

int tpm_dis_init(struct tpm_dis_init_para init_para_dis, aui_hdl *p_hdl)
{
    aui_hdl dis_hdl = 0;
    aui_attr_dis attr_dis;
    MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));
    attr_dis.uc_dev_idx = init_para_dis.ul_dis_type;

    if (aui_find_dev_by_idx(AUI_MODULE_DIS,
                            attr_dis.uc_dev_idx,
                            &dis_hdl))   /*if dis device has opened,return dis device handle*/
    {
        if (aui_dis_open(&attr_dis, &dis_hdl))
        {
            /*if dis hasn't opened,open dis device
                                                        and return dis device handle*/
            printf("\n aui_dis_open fail\n");
            return -1;
        }
    }

    /*Because logo will be showed after boot, but Linux does not have the last frame of
    backup at present. If there is no closing the layer when the first playing stream, the
    blurred screen will appear*/
    if (aui_dis_layer_enable(dis_hdl, init_para_dis.ul_dis_layer, 0))
    {
        printf("\n aui_dis_video_enable fail\n");
        return -1;
    }

    *p_hdl = dis_hdl;
    return 0;
}

int tpm_decv_init(struct tpm_decv_init_para init_para_decv, aui_hdl *p_hdl)
{
    aui_hdl decv_hdl = 0;
    (void)init_para_decv;
    aui_attr_decv attr_decv;
    enum aui_decv_format decv_type = init_para_decv.ul_video_type;
    MEMSET(&attr_decv, 0, sizeof(aui_attr_decv));
    attr_decv.uc_dev_idx = init_para_decv.video_id;
    attr_decv.dis_layer  = init_para_decv.ul_dis_layer;

    if (init_para_decv.callback != NULL)
    {
        attr_decv.callback_nodes[0].type       = AUI_DECV_CB_FIRST_SHOWED;
        attr_decv.callback_nodes[0].callback   = init_para_decv.callback;
        attr_decv.callback_nodes[0].puser_data = init_para_decv.puser_data;
        printf("DEBUG trace %d %p\n", __LINE__, init_para_decv.puser_data);
    }

    // DECV
    if (aui_find_dev_by_idx(AUI_MODULE_DECV, attr_decv.uc_dev_idx, &decv_hdl))
    {
        if (aui_decv_open(&attr_decv, &decv_hdl))
        {
            printf("\n aui_decv_open fail\n");
            return -1;
        }
    }

    printf("DEBUG trace %d init_para_decv.ul_video_type %lu\n",
           __LINE__,
           init_para_decv.ul_video_type);

    if (aui_decv_decode_format_set(decv_hdl, decv_type))
    {
        printf("\n aui_decv_decode_format_set fail\n");
        return -1;
    }

    printf(
        "DEBUG trace %d init_para_decv.ul_chg_mode %d\n", __LINE__, init_para_decv.ul_chg_mode);

    /*We have two modes for changing programs,one is AUI_DECV_CHG_BLACK which means it will
     *show black screen when changing programs.The other is AUI_DECV_CHG_STILL which means
     *it will show the last picture on the screen when changing programs.*/
    if (aui_decv_chg_mode_set(decv_hdl, init_para_decv.ul_chg_mode))
    {
        printf("\n aui_decv_chg_mode_set fail\n");
        return -1;
    }

    if (aui_decv_start(decv_hdl))
    {
        printf("\n aui_decv_start fail\n");
        return -1;
    }

    *p_hdl = decv_hdl;
    return 0;
}

int tpm_audio_init(struct tpm_audio_init_para init_para_audio,
                   aui_hdl *p_hdl_deca,
                   aui_hdl *p_hdl_snd)
{
    aui_hdl deca_hdl = 0;
    aui_hdl snd_hdl  = 0;
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;
    MEMSET(&attr_deca, 0, sizeof(aui_attr_deca));
    MEMSET(&attr_snd, 0, sizeof(aui_attr_snd));

    // SND
    if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl))
    {
        if (aui_deca_open(&attr_deca, &deca_hdl))
        {
            printf("\n aui_deca_open fail\n");
            return -1;
        }
    }

    printf("DEBUG trace %d init_para_audio.ul_audio_type %lu\n",
           __LINE__,
           init_para_audio.ul_audio_type);

    if (aui_deca_type_set(deca_hdl, init_para_audio.ul_audio_type))
    {
        printf("\n aui_deca_start fail\n");
        return -1;
    }

    if (aui_deca_start(deca_hdl, &attr_deca))
    {
        printf("\n aui_deca_start fail\n");
        return -1;
    }

    if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl))
    {
        if (aui_snd_open(&attr_snd, &snd_hdl))
        {
            printf("\n aui_snd_open fail\n");
            return -1;
        }
    }

    aui_snd_mute_set(snd_hdl, 0);
    aui_snd_vol_set(snd_hdl, init_para_audio.ul_volume);
    *p_hdl_deca = deca_hdl;
    *p_hdl_snd  = snd_hdl;
    return 0;
}

int tpm_av_init(struct ali_aui_hdls *p_handles, aui_hdl *p_hdl)
{
    aui_hdl av_hdl = 0;
    aui_attrAV attr_av;
    MEMSET(&attr_av, 0, sizeof(aui_attrAV));
    attr_av.st_av_info.b_audio_enable       = 1;
    attr_av.st_av_info.b_dmx_enable         = 1;
    attr_av.st_av_info.b_pcr_enable         = 1;
    attr_av.st_av_info.b_video_enable       = 1;
    attr_av.st_av_info.en_audio_stream_type = AUI_DECA_STREAM_TYPE_MPEG1;
    attr_av.st_av_info.en_spdif_type        = AUI_SND_OUT_MODE_DECODED;
    attr_av.st_av_info.en_video_stream_type = AUI_DECV_FORMAT_MPEG;
    attr_av.st_av_info.ui_audio_pid         = g_stream_pids[1];
    attr_av.st_av_info.ui_video_pid         = g_stream_pids[0];
    attr_av.st_av_info.ui_pcr_pid           = g_stream_pids[2];
    aui_attr_decv attr_decv;
    aui_attr_deca attr_deca;
    aui_attr_dmx attr_dmx;
    aui_attr_snd attr_snd;
    MEMSET(&attr_decv, 0, sizeof(aui_attr_decv));
    MEMSET(&attr_deca, 0, sizeof(aui_attr_deca));
    MEMSET(&attr_dmx, 0, sizeof(aui_attr_dmx));
    MEMSET(&attr_snd, 0, sizeof(aui_attr_snd));

    if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &p_handles->decv_hdl))
    {
        if (aui_decv_open(&attr_decv, &p_handles->decv_hdl))
        {
            printf("\n aui_decv_open fail\n");
            return -1;
        }
    }

    if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &p_handles->deca_hdl))
    {
        if (aui_deca_open(&attr_deca, &p_handles->deca_hdl))
        {
            printf("\n aui_deca_open fail\n");
            return -1;
        }
    }

    if (aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &p_handles->dmx_hdl))
    {
        if (aui_dmx_open(&attr_dmx, &p_handles->dmx_hdl))
        {
            printf("\n aui_dmx_open fail\n");
            return -1;
        }
    }

    if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &p_handles->snd_hdl))
    {
        if (aui_snd_open(&attr_snd, &p_handles->snd_hdl))
        {
            printf("\n aui_snd_open fail\n");
            return -1;
        }
    }

    attr_av.pv_hdl_decv = p_handles->decv_hdl;
    attr_av.pv_hdl_dmx  = p_handles->dmx_hdl;
    attr_av.pv_hdl_deca = p_handles->deca_hdl;
    attr_av.pv_hdl_snd  = p_handles->snd_hdl;

    if (aui_av_open(&attr_av, &av_hdl))
    {
        printf("\n aui_av_open fail\n");
        return -1;
    }

    if (aui_av_start(av_hdl))
    {
        printf("\n aui_av_start fail\n");
        return -1;
    }

#if 0

    if (aui_av_pause(av_hdl))
    {
        printf("\n aui_hdmi_open fail\n");
        return -1;
    }

    if (aui_av_resume(av_hdl))
    {
        printf("\n aui_hdmi_open fail\n");
        return -1;
    }

#endif
    *p_hdl = av_hdl;
    return 0;
}

int tpm_tsg_init(struct tpm_tsg_init_para *para, aui_hdl *p_hdl)
{
    aui_attr_tsg attr;
    MEMSET(&attr, 0, sizeof(aui_attr_tsg));
    attr.ul_tsg_clk = para->ul_tsg_clk;

    if (aui_tsg_open(&attr, p_hdl))
    {
        printf("aui_tsg_init error\n");
        aui_tsg_de_init(NULL, NULL);
        return -1;
    }

    attr.ul_bit_rate = para->ul_bit_rate;

    if (aui_tsg_start(*p_hdl, &attr))
    {
        printf("aui_tsg_start error\n");
        aui_tsg_close(*p_hdl);
        return 1;
    }

    return 0;
}

int ali_aui_init_para_for_test_nim(unsigned long *argc,
                                   char **argv,
                                   struct tpm_modules_init_para *init_para)
{
    enum aui_dis_format dis_format        = AUI_DIS_HD;         /*display format*/
    enum aui_en_decv_chg_mode change_mode = AUI_DECV_CHG_BLACK; /*the channel change mode*/
    unsigned long nim_dev                 = 0;
    unsigned long nim_demod_type          = AUI_NIM_QPSK;
    unsigned long freq                    = 0;
    unsigned long symb                    = 0;
    unsigned long v_type                  = 0;
    unsigned long a_type                  = 0;
    unsigned long band                    = 0; // AUI_NIM_BANDWIDTH_6_MHZ
    aui_nim_std nim_std                   = AUI_NIM_STD_DVBC_ANNEX_AC;
    unsigned long polar                   = AUI_NIM_POLAR_HORIZONTAL;
    unsigned char lnbf_type               = 0;
    long plp_index                        = 0;
    long plp_id                           = 0;
    unsigned int qam_mode                 = AUI_NIM_NOT_DEFINED;
    unsigned long plsn_value              = 0;
    unsigned long stream_id               = 0;
    long tsn                              = 0;
    unsigned char plhs                    = 0;
    unsigned long plhs_seq_31_0           = 0;
    unsigned long plhs_seq_63_32          = 0;
    unsigned long plhs_seq_89_64          = 0;
    unsigned long transport_type          = 0;
    unsigned char priority                = AUI_NIM_DVBT_PROFILE_HP;
    unsigned long i          = 0;

    /* extract para from argv*/
    if (*argc > 0)
    {
        nim_dev = atoi(argv[0]); /*nim device index,generally have tow device:0 and 1*/
    }

    for (i = 0; i < *argc; i++)
    {
        printf("ali_aui_init_para_for_test_nim argv[%d] = %s\n", i, argv[i]);
    }

    if (*argc > 1)
    {
        nim_demod_type = _to_aui_nim_demod_type(argv[1]);
        transport_type = _to_aui_transport_type(argv[1]);
    }

    if (*argc > 2)
    {
        freq = atoi(argv[2]);    /* channel frequency*/
    }

    if (*argc > 3)
    {
        symb = atoi(argv[3]); /* channel symbal*/

        if (nim_demod_type == AUI_NIM_QPSK)
        {
            symb = atoi(argv[3]); // symbol rate
        }
        else if (nim_demod_type == AUI_NIM_QAM)
        {
            symb = atoi(argv[3]); // symbol rate
        }
        else if (nim_demod_type == AUI_NIM_OFDM)
        {
            band = _to_aui_nim_bandwidth(argv[3]);
        }
    }

    if (*argc > 4)
    {
        g_stream_pids[0] = atoi(argv[4]) & AUI_INVALID_PID; /*video pid*/
    }

    if (*argc > 5)
    {
        g_stream_pids[1] = atoi(argv[5]) & AUI_INVALID_PID; /*audio pid*/
    }

    if (*argc > 6)
    {
        g_stream_pids[2] =
            atoi(argv[6]) & AUI_INVALID_PID; /*pcr(personal clock reference) pid*/
        g_stream_pids[3] = AUI_INVALID_PID;
        get_audio_description_pid(g_stream_pids + 3);
    }

    if (*argc > 7)
    {
        // v_type = atoi(argv[7]); /*video format*/
        v_type = _to_aui_decv(argv[7]);
    }

    if (*argc > 8)
    {
        // a_type = atoi(argv[8]); /*audio format*/
        a_type = _to_aui_deca(argv[8]);
    }

    if (*argc > 9)
    {
        // change_mode = (enum aui_en_decv_chg_mode)atoi(argv[9]); /*the channel change mode*/
        change_mode = _to_aui_decv_chg_mode(argv[9]);
    }

    if (*argc > 10)
    {
        switch (nim_demod_type)
        {
            case AUI_NIM_QPSK:
                polar = _to_aui_nim_polar(argv[10]);
                break;

            case AUI_NIM_QAM:
                nim_std = _to_aui_nim_std(argv[10]);
                break;

            case AUI_NIM_OFDM:
                nim_std = _to_aui_nim_std(argv[10]);
                break;

            default:
                break;
        }
    }

    if (*argc > 11)
    {
        lnbf_type = _to_aui_nim_std(argv[11]);
    }

    if (*argc > 12)
    {
        if (nim_demod_type == AUI_NIM_QPSK)
        {
            plsn_value = atoi(argv[11]);
            printf("PLSN: %ld\n", plsn_value);
        }
        else if (nim_demod_type == AUI_NIM_QAM)
        {
            qam_mode = _to_aui_nim_qam_mode(argv[11]);
            printf("qam_mode: %ld\n", qam_mode);
        }
        else if (nim_demod_type == AUI_NIM_OFDM)
        {
            plp_index = atoi(argv[11]); /*DVBT2 type*/
            printf("PLP index: %ld\n", plp_index);
            (void)plp_index;                               // just for build
            priority = _to_aui_nim_profile_type(argv[11]); /*DVBT proterty*/
        }
    }

    if (*argc > 13)
    {
        if (nim_demod_type == AUI_NIM_QPSK)
        {
            stream_id = atoi(argv[12]);
        }
        else if (nim_demod_type == AUI_NIM_QAM)
        {
            band = _to_aui_nim_bandwidth(argv[12]);
        }
        else if (nim_demod_type == AUI_NIM_OFDM)
        {
            plp_id = atoi(argv[12]);
            printf("PLP id: %ld\n", plp_id);
        }
    }

    printf("nim_std %d qam_mode %d, argc %d polar %d\n", nim_std, qam_mode, *argc, polar);

    if (*argc > 14)
    {
        if (nim_demod_type == AUI_NIM_QPSK)
        {
            tsn = atoi(argv[13]); /*DVB-S2X type*/
            printf("tsn: %ld\n", tsn);
        }
        else if (nim_demod_type == AUI_NIM_QAM)
        {
;
        }
        else if (nim_demod_type == AUI_NIM_OFDM)
        {
;
        }
    }

    if (*argc > 15)
    {
        plhs = atoi(argv[14]);
        printf("(DirecTV)PL Head scrambling: %s\n", plhs ? "True" : "False");

        if (plhs)
        {
            if (strlen(argv[15]) == 24)
            {
                char byte4[9] = {0};
                char *tailptr = NULL;
                memcpy(byte4, &argv[15][0], 8);
                plhs_seq_89_64 = strtoul(byte4, &tailptr, 16);
                memcpy(byte4, &argv[15][8], 8);
                plhs_seq_63_32 = strtoul(byte4, &tailptr, 16);
                memcpy(byte4, &argv[15][16], 8);
                plhs_seq_31_0 = strtoul(byte4, &tailptr, 16);
                printf("%08X %08X %08X\n", plhs_seq_89_64, plhs_seq_63_32, plhs_seq_31_0);
            }
            else
            {
                printf("PL Head scrambling sequence length error\n");
                printf("E.g. 02ffffffcccccccc88888888\n");
            }
        }
    }

    if (*argc > 16)
    {
        nim_std = _to_aui_nim_std(argv[16]);
        printf("Satellite signal generation: %d\n", nim_std);
    }

    MEMSET(init_para, 0, sizeof(struct tpm_modules_init_para));
    init_para->init_para_nim.ul_device    = nim_dev;
    init_para->init_para_nim.ul_freq      = freq;
    init_para->init_para_nim.ul_symb_rate = symb;
    init_para->init_para_nim.ul_nim_type  = nim_demod_type;
    init_para->init_para_nim.ul_freq_band = band;
    init_para->init_para_nim.ul_nim_std   = nim_std;
    init_para->init_para_tsi.ul_dmx_idx = AUI_TSI_OUTPUT_DMX_0;
    init_para->init_para_tsi.ul_tsi_id         = 0;
    init_para->init_para_tsi.ul_tis_port_idx   = AUI_TSI_CHANNEL_0;
    init_para->init_para_tsi.ul_nim_index      = nim_dev;
    init_para->init_para_tsi.ul_transport_type = transport_type;
    //  if (nim_dev == AUI_TSG_DEV) {
    /* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz
     * 24 -> TSG clock 4.16 MHz
     * 32 -> TSG clock 3.12 MHz
     * 48 -> TSG clock 2.08 MHz
     */
    //  init_para.init_para_tsg.ul_tsg_clk = 8;
    //  init_para.init_para_tsg.ul_bit_rate = 0; /* 0 for default bitrates */
    //}
    init_para->dmx_create_av_para.dmx_id            = 0;
    init_para->dmx_create_av_para.video_encode_type = v_type;
    init_para->dmx_create_av_para.video_pid         = g_stream_pids[0];
    init_para->dmx_create_av_para.audio_pid         = g_stream_pids[1];
    init_para->dmx_create_av_para.audio_desc_pid    = g_stream_pids[3];
    init_para->dmx_create_av_para.pcr_pid           = g_stream_pids[2];
    printf("\r\n pid=[%d][%d][%d][%d]",
           init_para->dmx_create_av_para.video_pid,
           init_para->dmx_create_av_para.audio_pid,
           init_para->dmx_create_av_para.audio_desc_pid,
           init_para->dmx_create_av_para.pcr_pid);
    init_para->init_para_dis.ul_dis_type = dis_format;
    init_para->init_para_decv.ul_video_type     = v_type;
    init_para->init_para_decv.ul_preview_enable = 0;
    init_para->init_para_decv.ul_chg_mode       = change_mode;
    init_para->init_para_audio.ul_volume     = 50;
    init_para->init_para_audio.ul_audio_type = a_type;
    // DVB-S
    init_para->init_para_nim.ul_polar       = polar;
    init_para->init_para_nim.lnbftype       = lnbf_type;
    init_para->init_para_nim.stream_id      = stream_id;
    init_para->init_para_nim.tsn            = tsn;
    init_para->init_para_nim.ul_plsn_value  = plsn_value;
    init_para->init_para_nim.plhs           = plhs;
    init_para->init_para_nim.plhs_seq_31_0  = plhs_seq_31_0;
    init_para->init_para_nim.plhs_seq_63_32 = plhs_seq_63_32;
    init_para->init_para_nim.plhs_seq_89_64 = plhs_seq_89_64;
    // DVB-C
    init_para->init_para_nim.qam_mode = qam_mode;
    // DVB-T
    init_para->init_para_nim.plp_id    = plp_id;
    init_para->init_para_nim.plp_index = plp_index;
    init_para->init_para_nim.priority  = priority;
    return 0;
}

#define TS_PACKET_SIZE (188)
#define TS_DSS_PACKET_SIZE (130)

int ali_aui_init_para_for_test_tsg(unsigned long *argc,
                                   char **argv,
                                   struct tpm_modules_init_para *init_para)
{
    // using the global variable g_stream_pids[]
    // unsigned short pids[3] = { 401 /* video */, 402 /* audio */, 401 /* pcr */ };
    unsigned long v_type                  = 0;
    unsigned long a_type                  = 0;
    enum aui_dis_format dis_format        = AUI_DIS_HD;
    enum aui_en_decv_chg_mode change_mode = AUI_DECV_CHG_BLACK; /*the channel change mode*/
    unsigned short tsg_clk_sel            = 8;
    unsigned long transport_type          = AUI_TSI_TRANSPORT_TYPE_TS;
    MEMSET(init_para, 0, sizeof(struct tpm_modules_init_para));
    /* extract para from argv*/
    g_stream_pids[0] = atoi(argv[1]); // vpid
    g_stream_pids[1] = atoi(argv[2]); // apid
    g_stream_pids[2] = atoi(argv[3]); // ppid
    v_type           = atoi(argv[4]);
    a_type           = atoi(argv[5]);
    change_mode      = atoi(argv[6]);

    if (*argc >= 8)
    {
        tsg_clk_sel = atoi(argv[7]);
    }

    if (*argc >= 10)
    {
        transport_type = _to_aui_transport_type(argv[9]);
        printf("transport_type: %lu, %s\n", transport_type, argv[9]);
    }

    switch (transport_type)
    {
        case AUI_TSI_TRANSPORT_TYPE_DSS:
            init_para->init_para_tsg.raw_packet_size = TS_DSS_PACKET_SIZE;
            break;

        default:
            init_para->init_para_tsg.raw_packet_size = TS_PACKET_SIZE;
            break;
    }

    printf("video_pid=%d,audio_pid=%d,pcr_pid=%d\n",
           g_stream_pids[0],
           g_stream_pids[1],
           g_stream_pids[2]);
    init_para->init_para_nim.ul_device = AUI_NIM_HANDLER_MAX; // AUI_TSG_DEV;
    init_para->init_para_tsi.ul_dmx_idx        = AUI_TSI_OUTPUT_DMX_0;
    init_para->init_para_tsi.ul_tsi_id         = 0;
    init_para->init_para_tsi.ul_tis_port_idx   = AUI_TSI_CHANNEL_0;
    init_para->init_para_tsi.ul_nim_index      = AUI_NIM_HANDLER_MAX; // for TSG play
    init_para->init_para_tsi.ul_transport_type = transport_type;
    /* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz
     * 24 -> TSG clock 4.16 MHz
     * 32 -> TSG clock 3.12 MHz
     * 48 -> TSG clock 2.08 MHz
     */
    init_para->init_para_tsg.ul_tsg_clk  = tsg_clk_sel;
    init_para->init_para_tsg.ul_bit_rate = 0; /* 0 for default bitrates */
    init_para->dmx_create_av_para.dmx_id            = 0;
    init_para->dmx_create_av_para.video_encode_type = v_type;
    init_para->dmx_create_av_para.video_pid         = g_stream_pids[0];
    init_para->dmx_create_av_para.audio_pid         = g_stream_pids[1];
    init_para->dmx_create_av_para.audio_desc_pid    = 0x1fff;
    init_para->dmx_create_av_para.pcr_pid           = g_stream_pids[2];
    printf("\r\n pid=[%d][%d][%d][%d]",
           init_para->dmx_create_av_para.video_pid,
           init_para->dmx_create_av_para.audio_pid,
           init_para->dmx_create_av_para.audio_desc_pid,
           init_para->dmx_create_av_para.pcr_pid);
    init_para->init_para_dis.ul_dis_type = dis_format;
    init_para->init_para_decv.ul_video_type     = v_type;
    init_para->init_para_decv.ul_preview_enable = 0;
    init_para->init_para_decv.ul_chg_mode       = change_mode;
    init_para->init_para_audio.ul_volume     = 50;
    init_para->init_para_audio.ul_audio_type = a_type;
    return 0;
}

int tpm_nim_deinit(aui_hdl hdl)
{
    int err = 0;

    if (hdl != NULL)
    {
        if (aui_nim_close(hdl) != AUI_RTN_SUCCESS)
        {
            printf("\r\n aui_nim_close error \n");
            err = 1;
        }
        else
        {
            printf("\r\n aui_nim_close OK \n");
        }
    }

    // if (aui_nim_de_init(NULL)) {
    //    printf("\r\n aui_nim_de_init error \n");
    //    err = 1;
    //}
    return err;
}

int tpm_tsi_deinit(aui_hdl hdl)
{
    if (aui_tsi_close(hdl))
    {
        printf("\r\n aui_tsi_close error \n");
        return 1;
    }

    return 0;
}

int tpm_tsg_deinit(aui_hdl hdl)
{
    if (aui_tsg_close(hdl))
    {
        printf("\r\n aui_tsg_close error \n");
        return 1;
    }

    return 0;
}

int tpm_dmx_deinit(aui_hdl hdl)
{
    int err = 0;

    if (aui_dmx_stop(hdl, NULL))
    {
        printf("\n aui_dmx_stop error \n");
        err = 1;
    }

    if (aui_dmx_close(hdl))
    {
        printf("\n aui_dmx_close error \n");
        err = 1;
    }

    return err;
}

int tpm_snd_deinit(aui_hdl hdl_deca, aui_hdl hdl_snd)
{
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;
    int err = 0;
    MEMSET(&attr_deca, 0, sizeof(aui_attr_deca));
    MEMSET(&attr_snd, 0, sizeof(aui_attr_snd));

    if (aui_deca_stop(hdl_deca, &attr_deca))
    {
        printf("\n aui_deca_close error \n");
        err = 1;
    }

#if 0

    if (aui_snd_stop(hdl_snd, &attr_snd))
    {
        printf("\n aui_snd_stop error \n");
        err = 1;
    }

#endif
    printf("\r\n close deca aui");

    if (aui_deca_close(hdl_deca))
    {
        printf("\n aui_deca_close error \n");
        err = 1;
    }

    printf("\r\n close snd aui");

    if (hdl_snd != NULL)
    {
        if (aui_snd_close(hdl_snd))
        {
            printf("\n aui_deca_close error \n");
            err = 1;
        }
    }

    return err;
}

int tpm_dis_deinit(aui_hdl *p_hdl)
{
    int err = 0;

    if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_HD, p_hdl))
    {
        printf("\n%s ->Cannot find dis_hd device\n", __FUNCTION__);
        err = 1;
    }

    if (p_hdl && aui_dis_close(NULL, p_hdl))
    {
        printf("\n aui_dis_close error \n");
        err = 1;
    }

    return err;
}

int tpm_decv_deinit(aui_hdl hdl)
{
    int err = 0;

    if (aui_decv_stop(hdl))
    {
        printf("\n aui_decv_stop error \n");
        err = 1;
    }

    if (aui_decv_close(NULL, &hdl))
    {
        printf("\n aui_decv_close error \n");
        err = 1;
    }

    return err;
}

int tpm_deinit(struct ali_aui_hdls *handles)
{
    printf("\r\n close nim aui");

    if (handles->nim_hdl && tpm_nim_deinit(handles->nim_hdl))
    {
        printf("\r\n tpm_nim_deinit failed!");
    }

    printf("\r\n close tsg aui");

    if (handles->tsg_hdl && tpm_tsg_deinit(handles->tsg_hdl))
    {
        printf("\r\n tpm_tsg_deinit failed!");
    }

    printf("\r\n close tsi aui");

    if (handles->tsi_hdl && tpm_tsi_deinit(handles->tsi_hdl))
    {
        printf("\r\n tpm_tsi_deinit failed!");
    }

    printf("\r\n close dmx aui");

    if (handles->dmx_hdl && tpm_dmx_deinit(handles->dmx_hdl))
    {
        printf("\r\n tpm_dmx_deinit failed!");
    }

    printf("\r\n close snd aui");

    if (handles->snd_hdl && tpm_snd_deinit(handles->deca_hdl, handles->snd_hdl))
    {
        printf("\r\n tpm_snd_deinit failed!");
    }

    printf("\r\n close decv aui");

    if (handles->decv_hdl && tpm_decv_deinit(handles->decv_hdl))
    {
        printf("\r\n tpm_decv_deinit failed!");
    }

    printf("\r\n close dis aui");

    if (handles->dis_hdl && tpm_dis_deinit(&handles->dis_hdl))
    {
        printf("\r\n tpm_dis_deinit failed!");
    }

    return AUI_RTN_SUCCESS;
}

