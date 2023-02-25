#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "my_module"
#define MESSAGE_BUFFER_SIZE 1024

static int Major;
static char message_buffer[MESSAGE_BUFFER_SIZE];
static int message_buffer_ptr = 0;

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "my_module: open()\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "my_module: release()\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    int bytes_to_read = message_buffer_ptr - *offset;

    if (bytes_to_read > length) {
        bytes_to_read = length;
    }

    if (bytes_to_read < 0) {
        return 0;
    }

    bytes_read = bytes_to_read - copy_to_user(buffer, message_buffer + *offset, bytes_to_read);
    *offset += bytes_read;

    printk(KERN_INFO "my_module: read() %d bytes\n", bytes_read);

    return bytes_read;
}

static ssize_t my_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
    int bytes_written = 0;
    int bytes_to_write = MESSAGE_BUFFER_SIZE - message_buffer_ptr;

    if (bytes_to_write > length) {
        bytes_to_write = length;
    }

    if (bytes_to_write < 0) {
        return 0;
    }

    bytes_written = bytes_to_write - copy_from_user(message_buffer + message_buffer_ptr, buffer, bytes_to_write);
    message_buffer_ptr += bytes_written;

    printk(KERN_INFO "my_module: write() %d bytes\n", bytes_written);

    return bytes_written;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init my_module_init(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &my_fops);

    if (Major < 0) {
        printk(KERN_ALERT "my_module: failed to register a major number\n");
        return Major;
    }

    printk(KERN_INFO "my_module: initialized with major number %d\n", Major);

    return 0;
}

static void __exit my_module_exit(void)
{
    unregister_chrdev(Major, DEVICE_NAME);

    printk(KERN_INFO "my_module: module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
