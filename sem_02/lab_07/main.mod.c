#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd9726f80, "module_layout" },
	{ 0x4caf37f7, "param_ops_int" },
	{ 0xf56eb709, "simple_statfs" },
	{ 0x51b23d27, "kill_block_super" },
	{ 0x1f290272, "unregister_filesystem" },
	{ 0x741bf5fb, "register_filesystem" },
	{ 0xdf6d705a, "kmem_cache_alloc" },
	{ 0xac799dd1, "kmem_cache_destroy" },
	{ 0x8fc2a09, "kmem_cache_create" },
	{ 0x37a0cba, "kfree" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xbfb1dbb9, "iput" },
	{ 0xe9bccd4a, "d_make_root" },
	{ 0xf997c6c9, "simple_dir_operations" },
	{ 0xd222b1ca, "simple_dir_inode_operations" },
	{ 0xa6a1a47d, "current_time" },
	{ 0xd879ad2a, "inode_init_owner" },
	{ 0xbfa7c6da, "init_user_ns" },
	{ 0x5dfb552f, "new_inode" },
	{ 0x2db6c90e, "mount_bdev" },
	{ 0xc5850110, "printk" },
	{ 0x18732c09, "kmem_cache_free" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "F0E9304A76E9D1A4B46E908");
