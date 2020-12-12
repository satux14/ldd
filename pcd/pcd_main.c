#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define PCD_MEM_SIZE 512

#undef pr_fmt
/* Decorate the print using pr_fmt macro */
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Pseudo device memory area */
char pcd_buf[PCD_MEM_SIZE];

/* Device number */
dev_t pcd_dev_num;

/* cdev */
struct cdev pcd_cdev;

loff_t pcd_lseek(struct file *fp, loff_t off, int whence) {
	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current pos: %lld\n", fp->f_pos);

	/* whence: SEEK_SET(pos=off), SEEK_CUR(pos=pos+off), SEEK_END(pos=MAX+off) */

	switch (whence) {
		case SEEK_SET:
			if ((off > PCD_MEM_SIZE) || off < 0)
				return -EINVAL;
			fp->f_pos = off;
			break;

		case SEEK_CUR:
			temp = fp->f_pos + off;
			if (temp > PCD_MEM_SIZE || temp < 0)
				return -EINVAL;

			fp->f_pos += off;
			break;

		case SEEK_END:
			temp = PCD_MEM_SIZE + off;
			if (temp > PCD_MEM_SIZE || temp < 0)
				return -EINVAL;

			fp->f_pos = PCD_MEM_SIZE + off;
			break;

		default:
			return -EINVAL;
	}
	pr_info("New pos: %lld\n", fp->f_pos);
    return 0;
}

ssize_t pcd_read(struct file *fp, char __user *buf, size_t count, loff_t *off) {
	pr_info("read requested for %zu bytes\n", count);
	pr_info("Current offset: %lld\n", *off);

	/* Adjust count */
	if ((*off + count) > PCD_MEM_SIZE)
		count = PCD_MEM_SIZE - *off;

	/* copy to user */
	if (copy_to_user(buf, &pcd_buf[*off], count)) {
		return -EFAULT;
	}

	/* Update the current file position */
	*off += count;

	pr_info("Number of bytes read: %zu\n", count);
	pr_info("New offset: %lld\n", *off);
	
	/* return number of bytes read */
    return count;
}

ssize_t pcd_write(struct file *fp, const char __user *buf, size_t count, loff_t *off) {
	pr_info("write requested for %zu bytes\n", count);
	pr_info("Current offset: %lld\n", *off);

	/* Adjust count */
	if ((*off + count) > PCD_MEM_SIZE)
		count = PCD_MEM_SIZE - *off;

	if (!count) {
		pr_err("No space left in device\n");
		return -ENOMEM;
	}

	if (copy_from_user(&pcd_buf[*off], buf, count)) {
		return -EFAULT;
	}

	/* update the current file position */
	*off += count;

	pr_info("Number of bytes write: %zu\n", count);
	pr_info("New offset: %lld\n", *off);
	
    return count;
}

int pcd_open(struct inode *in, struct file *fp) {
	pr_info("open requested \n");
	pr_info("open Successfull\n");
    return 0;
}

int pcd_release(struct inode *in, struct file *fp) {
	pr_info("release requested \n");
	pr_info("release Successfull\n");
    return 0;
}

/* file ops */
struct file_operations pcd_fops = 
{
	.open = pcd_open,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.release = pcd_release,
	.owner = THIS_MODULE,
};

struct class *pcd_class;

struct device *pcd_device;

static int __init pcd_driver_init(void) {
	int rc;

	/* Dynamically allocate the device number */
	rc = alloc_chrdev_region(&pcd_dev_num, 0, 1, "pcd_devices");
	if (rc < 0) {
		pr_err("Chrdev failed\n");
		goto out;
	}

	pr_info("Device number: Major:Minor=>%d:%d\n", MAJOR(pcd_dev_num), MINOR(pcd_dev_num));

	/* Initialize cdev */
	cdev_init(&pcd_cdev, &pcd_fops);

	/* Register cdev with VFS */
	pcd_cdev.owner = THIS_MODULE; /* cdev_init memset struct to 0 */
	rc = cdev_add(&pcd_cdev, pcd_dev_num, 1);
	if (rc < 0)
		pr_err("cdev add failed\n");
		goto unreg_region;

	/* create class for this device /sys/class/pcd_class */
	pcd_class = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(pcd_class)) {
		pr_err("Class creation failed\n");
		rc = PTR_ERR(pcd_class);
		goto del_cdev;
	}

	/* Create device */
	pcd_device = device_create(pcd_class, NULL, pcd_dev_num, NULL, "pcd");
	if (IS_ERR(pcd_device)) {
		pr_err("Device creation failed\n");
		rc = PTR_ERR(pcd_device);
		goto del_class;
	}

	pr_info("Module init successfull\n");

	return 0;

del_class:
	class_destroy(pcd_class);
del_cdev:
	cdev_del(&pcd_cdev);
unreg_region:
	unregister_chrdev_region(pcd_dev_num, 0);
out:
	return rc;

}

static void __exit pcd_driver_exit(void) {
	/* Unregister device */
	device_destroy(pcd_class, pcd_dev_num);

	/* Remove class */
	class_destroy(pcd_class);

	/* Delete cdev from VFS */
	cdev_del(&pcd_cdev);

	/* Unregister character region */
	unregister_chrdev_region(pcd_dev_num, 0);

	pr_info("Module unloaded successfully\n");
	return;
}

module_init(pcd_driver_init)
module_exit(pcd_driver_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sathish");
MODULE_DESCRIPTION("Pseudo char driver");

