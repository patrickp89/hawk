#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/proc_fs.h>
#include "hawk.h"




static const struct usb_device_id hawk_table[] = {
//	{ USB_DEVICE(VENDOR_ID,	PRODUCT_ID) },
	{ USB_DEVICE(0x0416,	0x9391) }, //WINBOND
	{ USB_DEVICE(0x1130,	0x0202) }, //Tenx Tech.
	{ }
};
MODULE_DEVICE_TABLE(usb, hawk_table);



struct usb_hawk {
	struct usb_device *udev;
	struct usb_interface *interface;
	struct kref kref;
};

static struct usb_driver hawk_driver;
static struct usb_hawk *dev;
static struct proc_dir_entry *hawk_proc_entry;



static char hawk_inc_buffer[2];
static int probe_counter;




static int send_ctrlmsg(struct usb_device *dest_dev, unsigned int dest_pipe,
						unsigned char b0, unsigned char b1, unsigned char b2,
						unsigned char b3, unsigned char b4)
{
	int i;
	unsigned char buffer[5];
	memset(buffer,0,5);

	buffer[0] = b0;
	buffer[1] = b1;
	buffer[2] = b2;
	buffer[3] = b3;
	buffer[4] = b4;

	i = usb_control_msg(dest_dev, dest_pipe, USB_REQ_SET_CONFIGURATION,
						USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE, 0x0300, 0, buffer, 5, 0);

//	printk(KERN_INFO "hawk: usb_control_msg(%02X %02X %02X %02X %02X) -> '%d'\n", b0, b1, b2, b3, b4, i);

	return i;
}


void rotate_left(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to rotate LEFT, the second byte must be set to 11101000:

	/*"5F E8 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE8, 0xE0, 0xFF, 0xFE);

	return;
}


void rotate_right(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to rotate RIGHT, the second byte must be set to 11100100:

	/*"5F E4 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE4, 0xE0, 0xFF, 0xFE);

	return;
}


void aim_up(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to aim UP, the second byte must be set to 11100010:

	/*"5F E2 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE2, 0xE0, 0xFF, 0xFE);

	return;
}


void aim_down(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to aim DOWN, the second byte must be set to 11100001:

	/*"5F E1 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE1, 0xE0, 0xFF, 0xFE);

	return;
}


void fire_missiles(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to FIRE missiles, the second byte must be set to 11110000:

	/*"5F F0 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xF0, 0xE0, 0xFF, 0xFE);

	return;
}


void stop_movement(struct usb_device *d_dev, unsigned int d_pipe)
{
	//to stop any movement, the second byte must be set to 11100000:

	/*"5F E0 E0 FF FE"*/
	send_ctrlmsg(d_dev, d_pipe, 0x5F, 0xE0, 0xE0, 0xFF, 0xFE);

	return;
}


int hawk_write_proc(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	memset(hawk_inc_buffer,' ',sizeof(hawk_inc_buffer));
	if (count <= sizeof(hawk_inc_buffer)) {
		if (copy_from_user(hawk_inc_buffer, buffer, count)) {
			return -EFAULT;
		}
		
		switch(hawk_inc_buffer[0])
		{
			case CONST_ROTATE_RIGHT:	stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										rotate_right(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			case CONST_ROTATE_LEFT:		stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										rotate_left(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			case CONST_AIM_UP:			stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										aim_up(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			case CONST_AIM_DOWN:		stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										aim_down(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			case CONST_FIRE_MISSILES:	stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										fire_missiles(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			case CONST_STOP_MOVEMENT:	stop_movement(dev->udev, usb_sndctrlpipe(dev->udev,0));
										break;

			default:					printk(KERN_INFO "hawk: wrong command issued\n");
										break;
		}
	}

	return count;
}


static void rm_hawk_proc_entry(void)
{
	if (hawk_proc_entry) {
			remove_proc_entry("hawk",NULL);
			hawk_proc_entry = NULL;
			printk(KERN_INFO "hawk: removed /proc/hawk\n");
	}
	
	return;
}


static int hawk_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	probe_counter++;
	if (probe_counter == 1) {
			dev = kzalloc(sizeof(*dev), GFP_KERNEL);
			if (!dev) {
					printk(KERN_ERR "hawk: kzalloc() failed!\n");
					if (dev) {
						kfree(dev);
					}
					return 1;
			} else {
					kref_init(&dev->kref);
					dev->udev = usb_get_dev(interface_to_usbdev(interface));
					dev->interface = interface;

					printk(KERN_INFO "hawk: device attached");
					if (!hawk_proc_entry) {
							hawk_proc_entry = create_proc_entry("hawk",0722,NULL);
							if(hawk_proc_entry) {
									hawk_proc_entry->write_proc = &hawk_write_proc;
									printk(KERN_INFO "hawk: created /proc/hawk\n");
							} else {
									printk(KERN_ERR "hawk: could not create /proc/hawk!\n");
									return 1;
							}
					} else {
							printk(KERN_ERR "hawk: /proc/hawk already existed! Clean up your /proc fs!\n");
							return 1;
					}
			}
	}

	return 0;
}


static void hawk_disconnect(struct usb_interface *interface)
{
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	kfree(dev);

	rm_hawk_proc_entry();
	probe_counter--;

	printk(KERN_INFO "hawk: device disconnected");
}


static struct usb_driver hawk_driver = {
	.name =			"hawk",
	.probe =		hawk_probe,
	.disconnect =	hawk_disconnect,
	.id_table =		hawk_table,
};


static int __init usb_hawk_init(void)
{
	int result;

	probe_counter = 0;
	result = usb_register(&hawk_driver);
	if (result) {
			printk(KERN_ERR "hawk: usb_register failed! Error number is: %d", result);
	} else {
			printk(KERN_INFO "hawk: module loaded\n");
	}

	return result;
}


static void __exit usb_hawk_exit(void)
{
	rm_hawk_proc_entry();
	usb_deregister(&hawk_driver);
	printk(KERN_INFO "hawk: module unloaded\n");
}


module_init(usb_hawk_init);
module_exit(usb_hawk_exit);

MODULE_LICENSE("GPL");
