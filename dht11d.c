#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define MAXTIMINGS 83
#define GPIO_SENSOR 4
int dht11_dat[5] = {0, 0, 0, 0, 0};

static void dht11_handler(struct work_struct *w);
static struct workqueue_struct *wq = 0;
static DECLARE_DELAYED_WORK(dht11_work, dht11_handler);
static unsigned long twosec;

static void dht11_handler(struct work_struct *w)
{
	unsigned int laststate = 1;
	unsigned int counter = 0;
	unsigned int i, j = 0;


	gpio_set_value(GPIO_SENSOR, 0);
	printk("sleep");
	msleep(18);
	printk("awake");
	gpio_set_value(GPIO_SENSOR, 1);
	for (i = 0; i < MAXTIMINGS; i++)
	{
		counter = 0;
		while (gpio_get_value(GPIO_SENSOR) == laststate)
		{
			counter++;
			mdelay(1);
			if (counter == 200) break;
		}
		laststate = gpio_get_value(GPIO_SENSOR);
		if (counter == 200) break;
		if ((i >= 4) && (i % 2 == 0))
		{
			dht11_dat[j/8] <<= 1;
			if (counter > 30) dht11_dat[j/8] |= 1;
			j++;
		}
	}
	printk("humidity = %d.%d %% Temperature = %d.%d *C \n", dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3]);
	queue_delayed_work(wq, &dht11_work, twosec);
}


static int __init dht11_init(void)
{
	gpio_set_value(GPIO_SENSOR, 1);
	twosec = msecs_to_jiffies(2000);
	if(!wq) wq = create_singlethread_workqueue("dht11q");
	if(wq) queue_delayed_work(wq, &dht11_work, twosec);

	return 0;
}

static void __exit dht11_exit(void)
{
	if (wq) destroy_workqueue(wq);
}

module_init(dht11_init);
module_exit(dht11_exit);

MODULE_DESCRIPTION("dht11");
MODULE_LICENSE("GPL");

