/************************************************************************************
** Copyright (C), 2008-2019, OPPO Mobile Comm Corp., Ltd
** VENDOR_EDIT
** File: oppo_sensor_devinfo.c
**
** Description:
**		Definitions for sensor devinfo.
**
** Version: 1.0
** Date created: 2019/09/14,17:02
** --------------------------- Revision History: ------------------------------------
* <version>		<date>		<author>		<desc>
**************************************************************************************/
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/smem.h>

#include <soc/oppo/oppo_project.h>
#include <soc/oppo/device_info.h>

#define SENSOR_DEVINFO_SYNC_TIME  10000 //10s

//SMEM_SENSOR = SMEM_VERSION_FIRST + 23,
#define SMEM_SENSOR 130
static int retry_count = 5;

struct devinfo {
	int id;
	char* ic_name;
	char* vendor_name;
};

struct sensor_info {
	int match_id;
	int als_lcd_type;
	int ps_type;
	int ps_cali_type;
};

enum VENDOR_ID {
	STK3A5X = 0,
};
static struct devinfo _info[] = {
	{STK3A5X,"stk33502","SensorTek"},
};
typedef enum {
	TM  = 0x01,
	BOE = 0x02,
} alsps_match_lcd_type;

struct sensor_info * g_chip = NULL;
int __attribute__((weak)) register_devinfo(char *name, struct manufacture_info *info) { return 1;}

struct delayed_work sensor_work;

static void oppo_sensor_parse_dts(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct sensor_info * chip = platform_get_drvdata(pdev);
	int rc = 0;
	int value = 0;

	pr_info("start \n");

	rc = of_property_read_u32(node, "match-id", &value);
	if (rc) {
		chip->match_id = STK3A5X;
	} else {
		chip->match_id = value;
	}

	rc = of_property_read_u32(node, "als-type-lcd", &value);
	if (rc) {
		pr_err("sensor_devinfo rc is ture\n");
		if(strstr(saved_command_line, "dsi_nt36672a_tm_vid_display"))
                 {
			chip->als_lcd_type = TM;
                 }
                else if(strstr(saved_command_line, "dsi_hx83112a_huaxing_vid_display") || strstr(saved_command_line, "dsi_hx83112a_dongshan_vid_display")
		   || strstr(saved_command_line, "dsi_hx83112a_boe_vid_display"))
                 {
                 	 chip->als_lcd_type = BOE;
                 }
                else
                 {
                  	chip->als_lcd_type = BOE;
                 }
        } else {
		pr_err("sensor_devinfo rc is fail\n");
		chip->als_lcd_type = value;
	}

	rc = of_property_read_u32(node, "ps-type", &value);
	if (rc) {
		chip->ps_type = 1;
	} else {
		chip->ps_type = value;
	}

	rc = of_property_read_u32(node, "ps-cali-type", &value);
	if (rc) {
		chip->ps_cali_type = 1;
	} else {
		chip->ps_cali_type = value;
	}

	pr_info("%s [%d %d %d %d] \n", __func__,
	chip->match_id,
	chip->als_lcd_type,
	chip->ps_type,
	chip->ps_cali_type);
}
static void sensor_dev_work(struct work_struct *work)
{
	int i = 0;
	int ret = 0;

	if (!g_chip) {
		pr_info("g_chip null\n");
		return;
	}

	pr_info("%s  match_id %d\n", __func__,g_chip->als_lcd_type);
	//need check sensor inited
	for (i = 0; i < sizeof(_info)/sizeof(struct devinfo); i++) {
		if (_info[i].id == g_chip->match_id) {
			do {
				ret = register_device_proc("Sensor_alsps", _info[i].ic_name, _info[i].vendor_name);
				pr_info("%s ret %d\n",__func__, ret);
			} while(--retry_count && ret);
			break;
		}
	}
}

static int oppo_devinfo_probe(struct platform_device *pdev)
{
	struct sensor_info * chip = NULL;
	size_t smem_size = 0;
	void *smem_addr = NULL;

	pr_info("%s call\n", __func__);

	smem_addr = qcom_smem_get(QCOM_SMEM_HOST_ANY,
		SMEM_SENSOR,
		&smem_size);
	if (IS_ERR(smem_addr)) {
		pr_err("unable to acquire smem SMEM_SENSOR entry\n");
		return -EPROBE_DEFER;
	}

	chip = (struct sensor_info *)(smem_addr);
	if (chip == ERR_PTR(-EPROBE_DEFER)) {
		chip = NULL;
		pr_err("unable to acquire entry\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev,chip);

	oppo_sensor_parse_dts(pdev);

	INIT_DELAYED_WORK(&sensor_work, sensor_dev_work);
	g_chip = chip;

	schedule_delayed_work(&sensor_work, msecs_to_jiffies(SENSOR_DEVINFO_SYNC_TIME));
	pr_info("%s success\n", __func__);
	return 0;
}

static int oppo_devinfo_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id of_drv_match[] = {
	{ .compatible = "sensor-devinfo"},
	{},
};
MODULE_DEVICE_TABLE(of, of_motor_match);

static struct platform_driver _driver = {
	.probe		= oppo_devinfo_probe,
	.remove		= oppo_devinfo_remove,
	.driver		= {
		.name	= "sensor_devinfo",
		.of_match_table = of_drv_match,
	},
};

static int __init oppo_devinfo_init(void)
{
	pr_info("oppo_devinfo_init call\n");

	platform_driver_register(&_driver);
	return 0;
}

core_initcall(oppo_devinfo_init);

MODULE_DESCRIPTION("sensor devinfo");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Murphy@oppo.com");
