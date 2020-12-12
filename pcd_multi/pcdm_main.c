#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define PCD_MAX_DEVICES 4

#define PCD_MEM_SIZE_DEV1 2048
#define PCD_MEM_SIZE_DEV2 512
#define PCD_MEM_SIZE_DEV3 1024
#define PCD_MEM_SIZE_DEV4 512

#undef pr_fmt
/* Decorate the print using pr_fmt macro */
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Pseudo device memory area */
char pcd_buf_dev1[PCD_MEM_SIZE_DEV1];
char pcd_buf_dev2[PCD_MEM_SIZE_DEV2];
char pcd_buf_dev3[PCD_MEM_SIZE_DEV3];
char pcd_buf_dev4[PCD_MEM_SIZE_DEV4];

struct pcd_private_data {
	struct module *owner;
	char *buffer;
	int size;
	const char *serial_number;
	int perm;
	struct cdev cdev; /* class dev */
};

struct pcd_driver_data {
	int tot_devices;
	dev_t pcd_dev_num; /* Device number */
	struct class *pcd_class;
	struct device *pcd_device;
	struct pcd_private_data dev_data[PCD_MAX_DEVICES];
};

struct pcd_driver_data pcd_data = {
	.tot_devices = PCD_MAX_DEVICES,
	.dev_data = {
		/* /dev/pcd-1 */
		[0] = {
			.buffer = pcd_buf_dev1,
			.size = PCD_MEM_SIZE_DEV1,
			.serial_number = "PCDABXYZ1",
			.perm = 0x1 /* RDONLY */
		},
		/* /dev/pcd-2 */
		[1] = {
			.buffer = pcd_buf_dev2,
			.size = PCD_MEM_SIZE_DEV2,
			.serial_number = "PCDABXYZ2",
			.perm = 0x10 /* WRONLY */
		},
		/* /dev/pcd-3 */
		[2] = {
			.buffer = pcd_buf_dev3,
			.size = PCD_MEM_SIZE_DEV3,
			.serial_number = "PCDABXYZ3",
			.perm = 0x11 /* RDWR */
		},
		/* /dev/pcd-4 */
		[3] = {
			.buffer = pcd_buf_dev4,
			.size = PCD_MEM_SIZE_DEV4,
			.serial_number = "PCDABXYZ4",
			.perm = 0x11 /* RDWR */
		},
	},
};

loff_t pcd_lseek(struct file *fp, loff_t off, int whence) {
	loff_t temp;
	struct pcd_private_data *pcd_data = (struct pcd_private_data *) fp->private_data;
	int max_size = pcd_data->size;

	pr_info("lseek requested \n");
	pr_info("Current pos: %lld\n", fp->f_pos);

	/* whence: SEEK_SET(pos=off), SEEK_CUR(pos=pos+off), SEEK_END(pos=MAX+off) */

	switch (whence) {
		case SEEK_SET:
			if ((off > max_size) || off < 0)
				return -EINVAL;
			fp->f_pos = off;
			break;

		case SEEK_CUR:
			temp = fp->f_pos + off;
			if (temp > max_size || temp < 0)
				return -EINVAL;

			fp->f_pos += off;
			break;

		case SEEK_END:
			temp = max_size + off;
			if (temp > max_size || temp < 0)
				return -EINVAL;

			fp->f_pos = max_size + off;
			break;

		default:
			return -EINVAL;
	}
	pr_info("New pos: %lld\n", fp->f_pos);
    return 0;
}

ssize_t pcd_read(struct file *fp, char __user *buf, size_t count, loff_t *off) {
	struct pcd_private_data *pcd_data = (struct pcd_private_data *) fp->private_data;
	int max_size = pcd_data->size;

	pr_info("read requested for %zu bytes\n", count);

	pr_info("Current offset: %lld\n", *off);

	/* Adjust count */
	if ((*off + count) > max_size)
		count = max_size - *off;

	/* copy to user */
	if (copy_to_user(buf, pcd_data->buffer + (*off), count)) {
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
	struct pcd_private_data *pcd_data = (struct pcd_private_data *) fp->private_data;
	int max_size = pcd_data->size;

	pr_info("write requested for %zu bytes\n", count);
	pr_info("Current offset: %lld\n", *off);

	/* Adjust count */
	if ((*off + count) > max_size)
		count = max_size - *off;

	if (!count) {
		pr_err("No space left in device\n");
		return -ENOMEM;
	}

	if (copy_from_user(pcd_data->buffer + (*off), buf, count)) {
		return -EFAULT;
	}

	/* update the current file position */
	*off += count;

	pr_info("Number of bytes write: %zu\n", count);
	pr_info("New offset: %lld\n", *off);
	
    return count;
}

#define RDONLY 	0x01
#define WRONLY 	0x10
#define RDWR	0x11

int file_perm_check(int dperm, int amode) {
	if (dperm == amode)
		return 0;

	/* Check for READ ONLY */
	if ((dperm == RDONLY) && ((amode & FMODE_READ) && !(amode & FMODE_WRITE)))
		return 0;

	/* Check for WRITE ONLY */
	if ((dperm == WRONLY) && ((amode & FMODE_WRITE) && !(amode & FMODE_READ)))
		return 0;
		
	return -EPERM;
}

int pcd_open(struct inode *in, struct file *fp) {
	int rc;
	int minor;
	struct pcd_private_data *pcd_data;

	pr_info("open requested \n");

	minor = MINOR(in->i_rdev);
	pr_info("Minor number: %d\n", minor);

	/* Device private data access from cdev */
	pcd_data = container_of(in->i_cdev, struct pcd_private_data, cdev);
	fp->private_data = pcd_data;

	rc = file_perm_check(pcd_data->perm, fp->f_mode);
	if (!rc) {
		pr_err("No persmission to open the device\n");	
	}

	pr_info("open Successfull\n");
    return rc;
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

static int __init pcd_driver_init(void) {
	int rc;
	int i;

	/* Dynamically allocate the device number */
	rc = alloc_chrdev_region(&pcd_data.pcd_dev_num, 0, PCD_MAX_DEVICES, "pcd_devices");
	if (rc < 0) {
		pr_err("Chrdev failed\n");
		goto out;
	}

	/* create class for this device /sys/class/pcd_class */
	pcd_data.pcd_class = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(pcd_data.pcd_class)) {
		pr_err("Class creation failed\n");
		rc = PTR_ERR(pcd_data.pcd_class);
		goto unreg_region;
	}

	for (i = 0; i < PCD_MAX_DEVICES; i++) {
		pr_info("Device number: Major:Minor=>%d:%d\n", MAJOR(pcd_data.pcd_dev_num + i), MINOR(pcd_data.pcd_dev_num + i));

		/* Initialize cdev */
		cdev_init(&pcd_data.dev_data[i].cdev, &pcd_fops);

		/* Register cdev with VFS */
		pcd_data.dev_data[i].owner = THIS_MODULE; /* cdev_init memset struct to 0 */
		rc = cdev_add(&pcd_data.dev_data[i].cdev, pcd_data.pcd_dev_num + i, 1);
		if (rc < 0) {
			pr_err("cdev add failed\n");
			goto del_cdev;
		}

		/* Create device */
		pcd_data.pcd_device = device_create(pcd_data.pcd_class, NULL, pcd_data.pcd_dev_num + i, NULL, "pcd-%d", i);
		if (IS_ERR(pcd_data.pcd_device)) {
			pr_err("Device creation failed\n");
			rc = PTR_ERR(pcd_data.pcd_device);
			goto del_class;
		}
	}

	pr_info("Module init successfull\n");

	return 0;

del_class:
del_cdev:
	for (; i >=0; i--) {
		device_destroy(pcd_data.pcd_class, pcd_data.pcd_dev_num+i);
		cdev_del(&pcd_data.dev_data[i].cdev);
	}
	class_destroy(pcd_data.pcd_class);
unreg_region:
	unregister_chrdev_region(pcd_data.pcd_dev_num, PCD_MAX_DEVICES);
out:
	return rc;
}

static void __exit pcd_driver_exit(void) {
	int i;
	for (i = 0; i < PCD_MAX_DEVICES; i++) {
		device_destroy(pcd_data.pcd_class, pcd_data.pcd_dev_num+i);
		cdev_del(&pcd_data.dev_data[i].cdev);
	}
	class_destroy(pcd_data.pcd_class);
	unregister_chrdev_region(pcd_data.pcd_dev_num, PCD_MAX_DEVICES);
	pr_info("Module unloaded successfully\n");

	return;
}

module_init(pcd_driver_init)
module_exit(pcd_driver_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sathish");
MODULE_DESCRIPTION("Pseudo char driver");

