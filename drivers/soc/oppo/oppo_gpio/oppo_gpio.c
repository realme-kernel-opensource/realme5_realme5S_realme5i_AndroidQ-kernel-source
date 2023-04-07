#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/platform_device.h>

#include <soc/oppo/oppo_project.h>

#define TAG	"oppo_gpio:"

#define __OF_GPIO_MATCH(p) \
	static const struct of_device_id p[] = { \
		{ .compatible = #p}, \
		{}, \
	}

#define __OPPO_PLATEFORM_DRIVER(p) \
	{ .probe = oppo_gpio_probe, \
	  .remove = oppo_gpio_remove \
	}

#define __OPPO_GPIO_DRIVER(p) \
	static struct platform_driver p = __OPPO_PLATEFORM_DRIVER(p)

#define OPPO_GPIO_DRIVER(d) \
	__OF_GPIO_MATCH(oppo_##d);\
	__OPPO_GPIO_DRIVER(d);

#define GPIO_DO_DRIVER_INIT(d) \
	do {\
		d.driver.name = #d;\
		d.driver.of_match_table = oppo_##d;\
	} while (0)

static int oppo_gpio_probe(struct platform_device *pdev)
{
	pr_info(TAG "%s dev:%s pcb_v:%d\n",
		__func__, pdev->name, get_PCB_Version());

	return 0;
}

static int oppo_gpio_remove(struct platform_device *pdev)
{
	pr_info(TAG "%s\n", __func__);

	return 0;
}

OPPO_GPIO_DRIVER(gpio_evt);
OPPO_GPIO_DRIVER(gpio_dvt);
OPPO_GPIO_DRIVER(gpio_pvt);

static int __init oppo_gpio_init(void)
{
	int pcb_v;

	pr_info(TAG "%s enter\n", __func__);

	pcb_v = get_PCB_Version();
	if (unlikely(pcb_v < 0 || pcb_v >= PCB_MAX)) {
		pr_err(TAG "%s erro,pcb_v:%d\n", __func__, pcb_v);
		return 0;
	}

	if (pcb_v < DVT1) {
		GPIO_DO_DRIVER_INIT(gpio_evt);
		platform_driver_register(&gpio_evt);
	} else if (pcb_v < PVT1) {
		GPIO_DO_DRIVER_INIT(gpio_dvt);
		platform_driver_register(&gpio_dvt);
	} else {
		GPIO_DO_DRIVER_INIT(gpio_pvt);
		platform_driver_register(&gpio_pvt);
	}

	return 0;
}

late_initcall_sync(oppo_gpio_init);
MODULE_DESCRIPTION("nc gpio config");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mofei@oppo.com");
