#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define JP_ARRAY_LEN 11

static int handler(void)
{
    int ret = 0;

    jprobe_return();
    return ret;
}

static struct jprobe jp_array[] = {
{ .entry = handler, .kp = {.symbol_name = "tp_4b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_8b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_16b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_32b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_64b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_128b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_192b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_256b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_512b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_768b"} },
{ .entry = handler, .kp = {.symbol_name = "tp_1kb"} },
};

static int __init jp_bench_init(void)
{
    int i, ret = 0;
    printk(KERN_INFO "Jprobe bench init\n");

    for(i = 0; i < JP_ARRAY_LEN; i++) {
        ret = register_jprobe(&jp_array[i]);
        if(ret < 0) {
            printk("Couldn't register jprobe for %s\n",
                   jp_array[i].kp.symbol_name);
            goto done;
            // TODO unregister previously registered probes
        }
    }

done:
    return ret;
}

static void __exit jp_bench_exit(void)
{
    int i;

    printk(KERN_INFO "Jprobe bench exit\n");
    for(i = 0; i < JP_ARRAY_LEN; i++) {
        unregister_jprobe(&jp_array[i]);
    }
}

module_init(jp_bench_init)
module_exit(jp_bench_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Kprobe benchmarking module");
