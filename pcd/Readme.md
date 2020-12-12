# Character Device Driver:  
## Device number    
	alloc_chrdev_region() : unregister_chrdev_region();   
		include/linux/fs.h  
	Registration with VFS for character (CDEV_ADD)  
	cdev_init();  
	cdev_add(); : cdev_del();  
		include/linux/cdev.h  
## Device files  
	class_create(); : class_destroy();     
	device_create() : device_destroy();  
		include/linux/device.h
## Implement file operations like open,close,read,write,seek etc.,  
	include/linux/fs.h  
	copy_to_user()  
	copy_from_user()  
		include/linux/uaccess.h  
## File Operations:  
## Device File Creation: udev or any commands/syscall  
	Inode creation in memory and set default variable to file ops  
## Device File Access:  
	File object creation: For every open call and it is destroyed when last close  
![Screenshot](fops1.jpg)
![Screenshot](udev.jpg)
### Verify sys files:
```
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# insmod pcd_main.ko
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# dmesg
[63594.577587] pcd_driver_init :Device number: Major:Minor=>237:0
[63594.577626] pcd_driver_init :Module init successfull
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# cd /sys/class/pc
pcd_class/ pci_bus/   pci_epc/
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# cd /sys/class/pc
pcd_class/ pci_bus/   pci_epc/
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# cd /sys/class/pcd_class/
root@sathish-VirtualBox:/sys/class/pcd_class# ls
pcd
root@sathish-VirtualBox:/sys/class/pcd_class# cd pcd
root@sathish-VirtualBox:/sys/class/pcd_class/pcd# ls
dev  power  subsystem  uevent
root@sathish-VirtualBox:/sys/class/pcd_class/pcd# cat dev
237:0
root@sathish-VirtualBox:/sys/class/pcd_class/pcd# cat uevent
MAJOR=237
MINOR=0
DEVNAME=pcd
root@sathish-VirtualBox:/sys/class/pcd_class/pcd# ls -lrth /dev/pcd
crw------- 1 root root 237, 0 Dec 12 10:37 /dev/pcd
root@sathish-VirtualBox:/sys/class/pcd_class/pcd# cd /home/sathish/workspace/ldd/pseudo_char_driver
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# rmmod pcd_main
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver# dmesg
[63594.577587] pcd_driver_init :Device number: Major:Minor=>237:0
[63594.577626] pcd_driver_init :Module init successfull
[63678.435237] pcd_driver_exit :Module unloaded successfully
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pseudo_char_driver#
```
### Running pcd kernel module
```
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pcd# insmod pcd_main.ko
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pcd# echo "Hello" > /dev/pcd
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pcd# cat /dev/pcd
Hello
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pcd# dmesg
[68052.691920] pcd_driver_init :Device number: Major:Minor=>509:0
[68052.692305] pcd_driver_init :Module init successfull
[68061.091650] pcd_open :open requested
[68061.091653] pcd_open :open Successfull
[68061.091662] pcd_write :write requested for 6 bytes
[68061.091663] pcd_write :Current offset: 0
[68061.091664] pcd_write :Number of bytes write: 6
[68061.091665] pcd_write :New offset: 6
[68061.091667] pcd_release :release requested
[68061.091668] pcd_release :release Successfull
[68064.053596] pcd_open :open requested
[68064.053599] pcd_open :open Successfull
[68064.053606] pcd_read :read requested for 131072 bytes
[68064.053607] pcd_read :Current offset: 0
[68064.053609] pcd_read :Number of bytes read: 512
[68064.053610] pcd_read :New offset: 512
[68064.053670] pcd_read :read requested for 131072 bytes
[68064.053671] pcd_read :Current offset: 512
[68064.053672] pcd_read :Number of bytes read: 0
[68064.053673] pcd_read :New offset: 512
[68064.053681] pcd_release :release requested
[68064.053682] pcd_release :release Successfull
root@sathish-VirtualBox:/home/sathish/workspace/ldd/pcd#
```
