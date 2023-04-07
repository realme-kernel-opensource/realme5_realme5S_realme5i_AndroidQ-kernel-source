#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/soc/qcom/smem.h>
#include <linux/seq_file.h>
#include <soc/oppo/oppo_project.h>
#include <soc/qcom/watchdog.h>
#include <linux/io.h>
#include <linux/input.h>

#define TAG	"dump:"

static void dump_post_process(void);

static int32_t dump_sequence = 0;
static bool door_open = false;

__attribute__((weak)) void oppo_switch_fulldump(bool open)
{
	return;
}

void oppo_force_to_fulldump(bool force)
{
	if (force && door_open) {
		msm_trigger_wdog_bite();
	}
}

static void dump_key_process(unsigned int code, int value)
{
	unsigned long timer_current = jiffies;
	static unsigned long timer_last = 0;

	if (value) {
		if (code == KEY_POWER) {
			/*reset sequence*/
			if (door_open) {
				oppo_switch_fulldump(false);
				pr_info(TAG "powerkey door closed\n");
			}
			dump_sequence = 0;
			timer_last = 0;
			door_open = false;
			return;
		}

		if (code == KEY_VOLUMEUP) {
			pr_debug(TAG "up pressed\n");
			dump_sequence = (dump_sequence << 1) | 0x01;
		}

		if (code == KEY_VOLUMEDOWN) {
			pr_debug(TAG "down pressed\n");
			dump_sequence = (dump_sequence << 1) & ~0x01;
		}

		if (door_open) {
			oppo_switch_fulldump(false);
			door_open = false;
			dump_sequence = 0;
			pr_info(TAG " door closed\n");
		}

		timer_current = jiffies;

		if (timer_last != 0) {
			if (time_after(timer_current, timer_last + msecs_to_jiffies(1000))){
				dump_sequence = 0;
				pr_debug(TAG "timeout to reset dump_sequence\n");
			}

			if((get_project() != OPPO_UNKNOWN) &&
				(get_project() == dump_sequence) &&
				door_open == false) {
				pr_info(TAG " door open, project:%d\n", get_project());
				oppo_switch_fulldump(true);
				door_open = true;
			}
		}

		timer_last = timer_current;
	}
}

void oppo_sync_key_event(unsigned int type, unsigned int code, int value)
{
	if (type != EV_KEY)
		return;

	if ((code != KEY_POWER) && (code != KEY_VOLUMEDOWN) && (code != KEY_VOLUMEUP))
		return;

	dump_key_process(code, value);

	if (door_open)
		dump_post_process();
}

static void dump_post_process(void)
{
	return;
}

