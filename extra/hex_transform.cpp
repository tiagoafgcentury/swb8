#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

// global variable to store the CRC table (faster computation)
static unsigned short crc_tabccitt[256];

void init_crcccitt_tab()
{
    for(int i = 0; i < 256; i++)
    {
        unsigned short crc = 0;
        unsigned short c = ((unsigned short) i) << 8;

        for(int j = 0; j < 8; j++)
        {
            if((crc ^ c) & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }

            c = c << 1;
        }

        crc_tabccitt[i] = crc;
    }
}

unsigned short update_crc_ccitt(unsigned short crc, char c)
{
    unsigned short short_c = 0x00ff & (unsigned short)c;
    unsigned short tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc_tabccitt[tmp];
    return crc;
}

unsigned char hextoc(const char *_byte)
{
    char b[3];
    b[0] = _byte[0];
    b[1] = _byte[1];
    b[2] = 0;
    return strtol(b, nullptr, 16);
}

void pk()
{
    init_crcccitt_tab();
    const char *ssv_length_str = "00000380";
    const char *pk_length_str = "00000180";
    const char *stb_ca_sn_str = "91771AE7";
    const char *pk_body_str = "030343EF6CBB46BBD25158F44B9328977370330FF64B3F6664D952F9D54E17A4775ADBD05E57AB9B88A09052973179A469C48FFE0702FA886F879B7F80FFBF5B567F0FE494BEE487F9F2108F104DB4DB2F1751D6F92917F652EB7646BF4E5F460289B5B1D52206933D9C669A5D1F42990450A8FBDB1D8A609EC28914B525EC318110E25641CBCD2BB4F0762573DD119154A8A2C972C08961FAB1BE1CC92AF3F52A0523F3F8760BD5E21E09A6D2FDD7A31A501847DA3E2871B927B1B7777B12F0FE2D369511F88F74D1639847B32B714797A7058B9A827DCB71EAE0D807C7DCF87970246140326ECFBE4FCDE308FF73770E9A73638DC9F037EE8617280BEA5ED093A22319F64EA2A6BC69280B48EE56A0CF501C51AC62F461BBB91CC8DF5565D2E87D9F0FFD5DEDCFD2E4B1B720D0E06B9887958C75CB978FF74A8572F5B2E6E8361B5836FD8016E9911A45DF76B2CA5071FF1E118D442B8072E1F9FBD7B8DE448CCD63A1009AA084C48911E23F605C1D54CC43C6F1BB5E80040000000000000000000000000050616972696E67204B6579204461746100C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0000000000000000000000000000000000000000000000000000000000000000000000000920C58B6792C175800E6F40990A39BA032FCDBBFECDFF27694B3AF0D10CF44CF82CB0FF1C19823762EC1BA7EFB8242C37571C6E8B5CBF3AF645D649F9576CA7E335EA958E8BC975A79AED674869937F12AA2E4557F694B271B0EE5403993DE60C5DB9C23EC335B8A46DEFC61827BBC1BFD9DF4DD042C9B8DA56D284986CEC686DFC72DF50D4B399085BB950D87E4124120E0157159953ECC4DA0DA53CE24A8900D7EE87A6BE3E4335378AF1DF0DE6F5B61BEEE346637A569DB212F0FF11900B0795BFC96D4BD9AC9BD200CB308CA4F96FBF6AE2488493776781814371FCEC6EF6F6ECF5DEC65996888D38EEA64EAC4063620A5B8723CAB4697702AEF0D028B44BBADFBA515A07384F74981A49FD54F02A5D85884FCC226879C346F6FE67D181E";
    const auto result_len = (strlen(ssv_length_str) + strlen(pk_length_str) + strlen(stb_ca_sn_str) + strlen(pk_body_str)) / 2;
    unsigned char result[result_len];
    auto p = &result[0];
    unsigned short calc_crc16 = 0;
    *p = hextoc(&ssv_length_str[0]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&ssv_length_str[2]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&ssv_length_str[4]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&ssv_length_str[6]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&pk_length_str[0]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&pk_length_str[2]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&pk_length_str[4]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&pk_length_str[6]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&stb_ca_sn_str[0]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&stb_ca_sn_str[2]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&stb_ca_sn_str[4]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;
    *p = hextoc(&stb_ca_sn_str[6]);
    calc_crc16 = update_crc_ccitt(calc_crc16, *p);
    p++;

    for(int i = 0; i < (result_len - 12); i++)
    {
        *p = hextoc(&pk_body_str[i * 2]);
        calc_crc16 = update_crc_ccitt(calc_crc16, *p);
        p++;
    }

    cout << hex << setfill('0') << setw(4) << calc_crc16 << endl;
    ofstream out("/tmp/0303.pk", std::ios_base::binary | std::ios_base::trunc);
    out.write(reinterpret_cast<const char *>(result) + 4, result_len - 4);
    out.close();
}

int main(int argc, char **argv)
{
    init_crcccitt_tab();
    unsigned short calc_crc16 = 0;

    for(int i = 1; i < argc; i++)
    {
        for(int j = 0; j < strlen(argv[i]); j += 2)
        {
            unsigned char c = hextoc(&(argv[i][j]));
            calc_crc16 = update_crc_ccitt(calc_crc16, c);
            cout << c;
        }
    }

    cerr << hex << setfill('0') << setw(4) << calc_crc16 << endl;
    return 0;
}
