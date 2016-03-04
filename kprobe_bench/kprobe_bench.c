#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define KP_ARRAY_LEN 11

static struct kprobe kp_array[] = {
{ . symbol_name = "tp_4b" },
{ . symbol_name = "tp_8b" },
{ . symbol_name = "tp_16b" },
{ . symbol_name = "tp_32b" },
{ . symbol_name = "tp_64b" },
{ . symbol_name = "tp_128b" },
{ . symbol_name = "tp_192b" },
{ . symbol_name = "tp_256b" },
{ . symbol_name = "tp_512b" },
{ . symbol_name = "tp_768b" },
{ . symbol_name = "tp_1kb" },
};

/* kprobe pre_handler: called just before the probed instruction is executed */
static int pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    int ret = 0;
    (void)p;
    (void)regs;

    return ret;
}

static int __init kp_bench_init(void)
{
    int i, ret = 0;
    printk(KERN_INFO "Kprobe bench init\n");

    for(i = 0; i < KP_ARRAY_LEN; i++) {
        kp_array[i].pre_handler = pre_handler;
        ret = register_kprobe(&kp_array[i]);
        if(ret < 0) {
            printk("Couldn't register probe for %s\n", kp_array[i].symbol_name);
            goto done;
            // TODO unregister previously registered probes
        }
    }

done:
    return ret;
}

static void __exit kp_bench_exit(void)
{
    int i;

    printk(KERN_INFO "Kprobe bench exit\n");
    for(i = 0; i < KP_ARRAY_LEN; i++) {
        unregister_kprobe(&kp_array[i]);
    }
}

module_init(kp_bench_init)
module_exit(kp_bench_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Kprobe benchmarking module");
