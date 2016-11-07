#include <errno.h>
#include <libusb.h>
#include <libusbi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <os/linux_usbfs.h>
#include <linux/usbdevice_fs.h>

// Interface, Endpoint, Alternate Setting
#define INTF 2
#define ALTS 1
#define EDPT 0x82
// Arbitrary values
#define BUFFER_CT 1
#define BUFFER_SIZE 192000 * 4
#define NUM_ISO_PACKETS 10 // arbitrary, may need to be lower
#define PKT_SIZE 192

#define DINFO printf("@%s:%d : %d\n", __FILE__, __LINE__, errno)
#define CHECK2(r) ({if (r) {printf("ERR (%d)", r); DINFO; return r;} else DINFO;})
#define CHECK() CHECK2(r)

libusb_device_handle *handle;
libusb_context * libusb_ctx;

int get_handle_using_fd(char * argv[]) // be sure to manually set permissions on dev file for this
{
	int r;
	// parse args
	int bus = atoi(argv[1]);
	int dev = atoi(argv[2]);
	char filename[21];
	snprintf(&filename[0], 21, "/dev/bus/usb/%03d/%03d", bus, dev);
	printf("%s %d %d\n", &filename[0], bus, dev);
	// Get context
	r = libusb_init(&libusb_ctx);
	CHECK();
	// Get fd
	int fd = open(&filename[0], O_RDWR);
	CHECK2(fd < 0);
	// return
	return 0;
}

int get_handle_using_libusb()
{
	return 0;
}

/* If args are given, they should be bus number and device number, expressed as strings */
int main(int argc, char * argv[])
{
	int r;
	if (argc > 2) {
		r = get_handle_using_fd(argv);
	} else {
		r = get_handle_using_libusb();
	}
}
