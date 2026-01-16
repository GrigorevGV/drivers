#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GrigorevGV");
MODULE_DESCRIPTION("A simple ubuntu module");
MODULE_VERSION("1.0");

static int __init simple_init(void) {
    printk(KERN_INFO "Hello from the mai 307 team\n");
    return 0;
}

static void __exit simple_exit(void) {
    printk(KERN_INFO "Goodbye from the mai 307 team!\n");
}

module_init(simple_init);
module_exit(simple_exit);

