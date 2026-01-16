#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x0c5a8f5b, "dev_get_by_name" },
	{ 0x3b9c121f, "sock_setsockopt" },
	{ 0xf21e8931, "kernel_bind" },
	{ 0x8fdc4711, "kthread_create_on_node" },
	{ 0xda80ecf4, "wake_up_process" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xb1e2caac, "sock_release" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x732dd6e2, "kthread_stop" },
	{ 0x329fc928, "in4_pton" },
	{ 0xa53f4e29, "memcpy" },
	{ 0x123f1a55, "kernel_sendmsg" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0xf77cdcec, "kernel_recvmsg" },
	{ 0x67628f51, "msleep" },
	{ 0xc2614bbe, "param_ops_int" },
	{ 0xc2614bbe, "param_ops_charp" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x44a495af, "init_net" },
	{ 0x3309544e, "sock_create_kern" },
	{ 0xba157484, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x0c5a8f5b,
	0x3b9c121f,
	0xf21e8931,
	0x8fdc4711,
	0xda80ecf4,
	0xd272d446,
	0xb1e2caac,
	0xd272d446,
	0x732dd6e2,
	0x329fc928,
	0xa53f4e29,
	0x123f1a55,
	0xe54e0a6b,
	0x5e505530,
	0xf77cdcec,
	0x67628f51,
	0xc2614bbe,
	0xc2614bbe,
	0xd272d446,
	0xe8213e80,
	0x44a495af,
	0x3309544e,
	0xba157484,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"dev_get_by_name\0"
	"sock_setsockopt\0"
	"kernel_bind\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"__x86_return_thunk\0"
	"sock_release\0"
	"__stack_chk_fail\0"
	"kthread_stop\0"
	"in4_pton\0"
	"memcpy\0"
	"kernel_sendmsg\0"
	"__fortify_panic\0"
	"kthread_should_stop\0"
	"kernel_recvmsg\0"
	"msleep\0"
	"param_ops_int\0"
	"param_ops_charp\0"
	"__fentry__\0"
	"_printk\0"
	"init_net\0"
	"sock_create_kern\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FD9DD48EA2937A4C79ABB77");
