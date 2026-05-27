#include "hal/mb_sound.h"

#include <aui_snd.h>
#include <aui_deca.h>
#include "mb_ali_globals.h"

#include "common/mb_globals.h"

namespace {

uint32_t sin_2000_sap[24] =
{
    0X2120FB83, 0X40000000, 0X5A82799A, 0X6ED9EBA1, 0X7BA3751D, 0X7FFFFFFF,
    0X7BA3751D, 0X6ED9EBA1, 0X5A82799A, 0X40000000, 0X2120FB83, 0X00000000,
    0XDEDF047D, 0XC0000000, 0XA57D8666, 0X9126145F, 0X845C8AE3, 0X80000000,
    0X845C8AE3, 0X9126145F, 0XA57D8666, 0XC0000000, 0XDEDF047D, 0X00000000,
};
uint16_t sin_2000_sap_size = sizeof(sin_2000_sap);

mb::Sound *s_instance { nullptr };

}

namespace mb {

constexpr auto MIN_VOLUME = 0;
constexpr auto MAX_VOLUME = 100;

struct Sound::Data
{
    aui_hdl handle { nullptr };
    aui_hdl hnd_deca { nullptr };
};

Sound *Sound::get_instance()
{
    return s_instance;
}


Sound::Sound():
    m_p(std::make_unique<Data>())
{
    s_instance = this;
    ALI_EXEC(aui_snd_init(nullptr, nullptr));
    aui_attr_snd attr_snd;
    MB_ZERO(attr_snd);
    ALI_EXEC(aui_snd_open(&attr_snd, &m_p->handle));
    ALI_EXEC(aui_deca_init(nullptr, nullptr));
    aui_attr_deca attr_deca;
    MB_ZERO(attr_deca);
    ALI_EXEC(aui_deca_open(&attr_deca, &m_p->hnd_deca));

}

Sound::~Sound()
{
    stop_tone();
    ALI_EXEC(aui_snd_close(m_p->handle));
    ALI_EXEC(aui_snd_de_init(nullptr, nullptr));
    ALI_EXEC(aui_deca_close(m_p->hnd_deca));
    ALI_EXEC(aui_snd_de_init(nullptr, nullptr));
    s_instance = nullptr;
}

void Sound::increment_volume(int _volume)
{
    auto handle = m_p->handle;
    unsigned char vol{ 0 };
    ALI_EXEC(aui_snd_vol_get(handle, &vol));
    _volume += (int)vol;
    set_volume(std::max(_volume, 0));
}

void Sound::set_volume(Volume_t _volume)
{
    auto handle = m_p->handle;

    if(_volume < MIN_VOLUME)
    {
        _volume = MIN_VOLUME;
    }
    else if(_volume > MAX_VOLUME)
    {
        _volume = MAX_VOLUME;
    }

    ALI_EXEC(aui_snd_mute_set(m_p->handle, false));
    auto ret = aui_snd_vol_set(handle, _volume);

    if(ret != AUI_RTN_SUCCESS)
    {
        // This is OK, will fail if we set volume when muted
        DEBUG_MSG(HAL, ERROR, "Set volume FAILED: " << ret << "\n");
    }
}

Volume_t Sound::get_volume()
{
    unsigned char vol{ 0 };
    ALI_EXEC(aui_snd_vol_get(m_p->handle, &vol));
    return vol;
}

bool Sound::mute_toggle()
{
    unsigned char mute{ 0 };
    ALI_EXEC(aui_snd_mute_get(m_p->handle, &mute));

    if(mute)
    {
        mute = 0;
    }
    else
    {
        mute = 1;
    }

    ALI_EXEC(aui_snd_mute_set(m_p->handle, mute));
    return mute;
}

bool Sound::mute_state()
{
    unsigned char mute{ 0 };
    ALI_EXEC(aui_snd_mute_get(m_p->handle, &mute));
    return mute;
}

void Sound::set_tone(double snr)
{
    ALI_EXEC(aui_deca_bee_tone_start(m_p->hnd_deca, 50, (int*)sin_2000_sap, sin_2000_sap_size));
}

void Sound::stop_tone()
{
    ALI_EXEC(aui_deca_bee_tone_stop(m_p->hnd_deca));
}

} // namespace mb
