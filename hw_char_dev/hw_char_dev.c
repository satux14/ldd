#include<linux/module.h>

static int __init hello_world_init(void) {
	pr_info("Hello World!!!\n");
	return 0;
}

static void __exit hello_world_cleanup(void) {
	pr_info("Good Bye World!!!\n");
	return;
}

module_init(hello_world_init)
module_exit(hello_world_cleanup)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sathish");
MODULE_DESCRIPTION("Hello World kernel module");
MODULE_INFO(message, "Static info to module");
