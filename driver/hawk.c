// SPDX-License-Identifier: GPL-3.0+
/*
  This driver is based on Greg Kroah-Hartman's USB Skeleton driver.
  See: https://github.com/torvalds/linux/blob/master/drivers/usb/usb-skeleton.c
*/

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/proc_fs.h>
#include "hawk.h"

/*
  The module's device table (used for hotplugging):
*/
static const struct usb_device_id hawk_table[] = {
		//	{ USB_DEVICE(VENDOR_ID,	PRODUCT_ID) },
		{USB_DEVICE(0x0416, 0x9391)}, //WINBOND
		{USB_DEVICE(0x1130, 0x0202)}, //Tenx Tech.
		{}
};
MODULE_DEVICE_TABLE(usb, hawk_table);

/*
  A struct holding all device specific structures:
*/
struct usb_hawk
{
	struct usb_device *udev;
	struct usb_interface *interface;
	struct kref kref;
};

/*
  Variables and structures used:
*/
static struct usb_driver hawk_driver;
static struct usb_hawk *dev;
static struct proc_dir_entry *hawk_proc_entry;
static char hawk_inc_buffer[2];
static int probe_counter;

/*
  Function declarations:
*/
static int send_ctrlmsg(struct usb_device *dest_dev, unsigned int dest_pipe,
												unsigned char b0, unsigned char b1, unsigned char b2,
												unsigned char b3, unsigned char b4);

void rotate_left(struct usb_device *d_dev, unsigned int d_pipe);
void rotate_right(struct usb_device *d_dev, unsigned int d_pipe);
void aim_up(struct usb_device *d_dev, unsigned int d_pipe);
void aim_down(struct usb_device *d_dev, unsigned int d_pipe);
void fire_missiles(struct usb_device *d_dev, unsigned int d_pipe);
void stop_movement(struct usb_device *d_dev, unsigned int d_pipe);

static ssize_t hawk_write_proc(struct file *filp, const char __user *buffer, size_t len, loff_t *off);
static void rm_hawk_proc_entry(void);
static int hawk_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void hawk_disconnect(struct usb_interface *interface);
static int __init usb_hawk_init(void);
static void __exit usb_hawk_exit(void);

// The driver's proc_fs file operations:
static struct file_operations fops = {
		.owner = THIS_MODULE,
		.write = hawk_write_proc,
};

/*
  The driver's structure:
*/
static struct usb_driver hawk_driver = {
		.name = "hawk",
		.probe = hawk_probe,
		.disconnect = hawk_disconnect,
		.id_table = hawk_table,
};

/*
  Send a control message to the USB device.
*/
static int send_ctrlmsg(struct usb_device *dest_dev, unsigned int dest_pipe,
												unsigned char b0, unsigned char b1, unsigned char b2,
												unsigned char b3, unsigned char b4)
{
	unsigned char buffer[5];
	memset(buffer, 0, 5);

	buffer[0] = b0;
	buffer[1] = b1;
	buffer[2] = b2;
	buffer[3] = b3;
	buffer[4] = b4;

	return usb_control_msg(dest_dev, dest_pipe, USB_REQ_SET_CONFIGURATION,
											USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE, 0x0300, 0, buffer, 5, 0);
}

/*
  Rotate the missile launcher counter-clockwise.
*/
void rotate_left(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to rotate LEFT, the second byte must be set to 11101000 ("5F E8 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE8, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Rotate the missile launcher clockwise.
*/
void rotate_right(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to rotate RIGHT, the second byte must be set to 11100100 ("5F E4 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE4, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Let the missile launcher's turrets aim up.
*/
void aim_up(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to aim UP, the second byte must be set to 11100010 ("5F E2 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE2, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Let the missile launcher's turrets aim down.
*/
void aim_down(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to aim DOWN, the second byte must be set to 11100001 ("5F E1 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE1, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Trigger a 'missile launch'.
*/
void fire_missiles(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to FIRE missiles, the second byte must be set to 11110000 ("5F F0 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xF0, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Force the missile launcher to stop any action it is performing.
*/
void stop_movement(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to stop any movement, the second byte must be set to 11100000 ("5F E0 E0 FF FE"):
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE0, 0xE0, 0xFF, 0xFE);
	return;
}

/*
  Handle write operations to the proc_fs entry corresponding to this driver instance.
*/
static ssize_t hawk_write_proc(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
	memset(hawk_inc_buffer, ' ', sizeof(hawk_inc_buffer));
	if (len <= sizeof(hawk_inc_buffer))
	{
		if (copy_from_user(hawk_inc_buffer, buffer, len))
		{
			return -EFAULT;
		}

		switch (hawk_inc_buffer[0])
		{
		case CONST_ROTATE_RIGHT:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			rotate_right(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		case CONST_ROTATE_LEFT:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			rotate_left(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		case CONST_AIM_UP:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			aim_up(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		case CONST_AIM_DOWN:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			aim_down(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		case CONST_FIRE_MISSILES:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			fire_missiles(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		case CONST_STOP_MOVEMENT:
			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev, 0));
			break;

		default:
			printk(KERN_INFO "hawk: wrong command issued\n");
			break;
		}
	}

	return len;
}

/*
  Remove the proc_fs entry.
*/
static void rm_hawk_proc_entry(void)
{
	if (hawk_proc_entry)
	{
		remove_proc_entry("hawk", NULL);
		hawk_proc_entry = NULL;
		printk(KERN_INFO "hawk: removed /proc/hawk\n");
	}

	return;
}

/*
  Allocate kernel memory and create the proc_fs entry.
*/
static int hawk_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	probe_counter++;
	if (probe_counter == 1)
	{
		dev = kzalloc(sizeof(*dev), GFP_KERNEL);
		if (!dev)
		{
			printk(KERN_ERR "hawk: kzalloc() failed!\n");
			if (dev)
			{
				kfree(dev);
			}
			return 1;
		}
		else
		{
			kref_init(&dev->kref);
			dev->udev = usb_get_dev(interface_to_usbdev(interface));
			dev->interface = interface;

			printk(KERN_INFO "hawk: device attached");
			if (!hawk_proc_entry)
			{
				hawk_proc_entry = proc_create("hawk", 0722, NULL, &fops);
				if (hawk_proc_entry)
				{
					printk(KERN_INFO "hawk: created /proc/hawk\n");
				}
				else
				{
					printk(KERN_ERR "hawk: could not create /proc/hawk!\n");
					return 1;
				}
			}
			else
			{
				printk(KERN_ERR "hawk: /proc/hawk already existed! Clean up your /proc fs!\n");
				return 1;
			}
		}
	}

	return 0;
}

/*
  Unregister the device from the USB subsystem when the device is unplugged.
*/
static void hawk_disconnect(struct usb_interface *interface)
{
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	kfree(dev);

	rm_hawk_proc_entry();
	probe_counter--;

	printk(KERN_INFO "hawk: device disconnected");
}

/*
  Register the driver with the USB subsystem when the module is loaded into the kernel.
*/
static int __init usb_hawk_init(void)
{
	int result;

	probe_counter = 0;
	result = usb_register(&hawk_driver);
	if (result)
	{
		printk(KERN_ERR "hawk: usb_register failed! Error number is: %d", result);
	}
	else
	{
		printk(KERN_INFO "hawk: module loaded\n");
	}

	return result;
}
module_init(usb_hawk_init);

/*
  Unregister the driver from the USB subsystem when the module is unloaded from the kernel.
*/
static void __exit usb_hawk_exit(void)
{
	rm_hawk_proc_entry();
	usb_deregister(&hawk_driver);
	printk(KERN_INFO "hawk: module unloaded\n");
}
module_exit(usb_hawk_exit);

/*
  This is a non-proprietary driver module :) in version 1.1.
*/
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");
