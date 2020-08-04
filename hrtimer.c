/*
 * Measure the timer-interrupt latency by Cyclictest.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/moduleparam.h>

#define DEVICE_NAME "hrtimer"
#define DEVICE "/dev/hrtimer"
#define US_TO_NS(x) (x * 1000)
#define DEFAULT_DELAY_US 200

static unsigned long delay_us = DEFAULT_DELAY_US;
module_param_named(delay_us, delay_us, ulong, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(delay_us, "Delay is 200 (us) by default");

static struct hrtimer timer;

static int my_open(struct inode *iobj, struct file *fobj)
{
        return 0;
}

static int my_release(struct inode *iobj, struct file *fobj)
{
        return 0;
}

static long my_ioctl(struct file *fobj,
                unsigned int cmd, unsigned long arg)
{
        return 0;
}

static struct file_operations misc_device_operations = {
        .owner = THIS_MODULE,
        .open = my_open,
        .release = my_release,
        .unlocked_ioctl = my_ioctl,
};

static struct miscdevice misc_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = DEVICE_NAME,
        .fops = &misc_device_operations
};

enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer)
{
        ktime_t interval;
        ktime_t now;
        ktime_t softexpires;
        u64 overruns;

        now = ktime_get();
        softexpires = timer->_softexpires;
        trace_printk("latency: %llu\t%llu\t%llu\n",
                     now, softexpires, ktime_sub(now, softexpires));

        interval = ktime_set(0, US_TO_NS(delay_us));
        overruns = hrtimer_forward(timer, now , interval);
        //trace_printk("overrun: %llu\n", overruns);

        return HRTIMER_RESTART;
}

void initialize_hrtimer(struct hrtimer *timer)
{
        hrtimer_init(timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        timer->function = &my_hrtimer_callback;
}

void start_hrtimer(struct hrtimer *timer)
{
        ktime_t delay;

        delay= ktime_set(0, US_TO_NS(delay_us));
        hrtimer_start(timer, delay, HRTIMER_MODE_REL);
}

static int __init my_initialize(void) {
        int is_error;

        is_error = misc_register(&misc_device);

        if (is_error < 0) {
                pr_alert("Failed to register %s\n", __this_module.name);
                return -is_error;
        } else {
                pr_info("Register %s\n", __this_module.name);
                pr_info("Start %llu\n", ktime_get());

                initialize_hrtimer(&timer);
                start_hrtimer(&timer);
        }
        return 0;
}

static void __exit my_exit(void) {
        int ret;

        ret = hrtimer_cancel(&timer);

        if (ret)
                pr_info("timer is cancelled\n");

        misc_deregister(&misc_device);

        pr_info("End %llu\n", ktime_get());
        pr_info("%s bye!\n", __this_module.name);
}

module_init(my_initialize);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Cheng");
MODULE_DESCRIPTION("cyclictest");
