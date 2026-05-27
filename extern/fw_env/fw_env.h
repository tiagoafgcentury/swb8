/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */

//#define CONFIG_ENV_UBI
//#define CONFIG_ENV_VOL "/dev/ubi0_2"
//#define CONFIG_ENV_UBI "/dev/ubi0"

//#define CONFIG_FILE     "/etc/fw_env.config"

#ifdef __cplusplus
extern "C" {
#endif

#define BOOTENV_IMG_NAME  "bootenv"
#define BOOTENV_BAK_IMG_NAME  "bootenv_bak"
#define UBI_MAX_VOLUMES     128

#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd1"
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000
#define DEVICE1_ENVSECTORS     2
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#define DEVICE2_ENVSECTORS     2

#define UBOOT_BOOTENV_SIZE 0x10000

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

int fw_printenv(int argc, char *argv[]);
char *fw_getenv(const char *name);
const char *fw_getenv_safe(const char *name);
int fw_setenv(int argc, char *argv[]);
int fw_parse_script(char *fname);
int fw_env_open(void);
int fw_env_write_int(const char *name, int value);
int fw_env_write(const char *name, const char *value);
int fw_env_close(void);
unsigned	long  crc32	 (unsigned long, const unsigned char *, unsigned);

#ifdef CONFIG_ENV_UBI
struct ubi_vol_info{
	char mtdname[128];
	unsigned long is_co_rootfs;
	unsigned long vol_id;
};
extern int get_volid_by_name(const char *vol_name);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
