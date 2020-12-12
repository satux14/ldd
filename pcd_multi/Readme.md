# Device driver for mulitple devices
- Create multiple device private structure
- Create a class
- cdev_add for each device: VFS hook
- device_create for each device: sys device creation

## Notes
Get the device data from cdev and container_of.
cdev is part of device private structure (note it is not a pointer, it is value inside the structure).
So we can get access to the structure using container_of helper function.

File pointer has private_data pointer which can be used to share the
device data pointer to all the further file operation calls.

## Permission:
FMODE_READ
FMODE_WRITE
File pointer has f_mode to get open mode information
