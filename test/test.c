#include <errno.h>
#include <string.h>
#include <libusb.h>
#include <libusbi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <os/linux_usbfs.h>
#include <linux/usbdevice_fs.h>

#include "copy.h"

// VID & PID (used w/out fd)
#define VID 0x0582
#define PID 0x0073
// Interface, Endpoint, Alternate Setting (used w/ fd)
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

libusb_device *dev;
libusb_device_handle *handle;
libusb_context * libusb_ctx;
unsigned long session_id;

static void capture_callback(struct libusb_transfer *transfer)
{
}

static struct libusb_transfer *alloc_capture_transfer(void)
{
	int bufflen = PKT_SIZE * NUM_ISO_PACKETS;
	int i;
	struct libusb_transfer *transfer = libusb_alloc_transfer(NUM_ISO_PACKETS);
	DINFO;
	if (!transfer) exit(1);
	transfer->dev_handle = handle;
	transfer->endpoint = EDPT;
	transfer->type = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
	transfer->timeout = 5000;
	transfer->buffer = malloc(bufflen);
	transfer->length = bufflen;
	transfer->callback = capture_callback;
	transfer->num_iso_packets = NUM_ISO_PACKETS;

	for (i = 0; i < NUM_ISO_PACKETS; i++) {
		transfer->iso_packet_desc[i].length = PKT_SIZE;
	}
	return transfer;
}

int get_handle_using_fd(char * argv[]) // be sure to manually set permissions on dev file for this
{
	int r;
	// parse args
	int busnum = atoi(argv[1]);
	int devnum = atoi(argv[2]);
	char filename[21];
	snprintf(&filename[0], 21, "/dev/bus/usb/%03d/%03d", busnum, devnum);
	printf("%s %d %d\n", &filename[0], busnum, devnum);
	// Get fd
	int fd = open(&filename[0], O_RDWR);
	if (fd < 0) printf("No file descriptor obtained. You may have forgotten to:\n\tchmod o+rw %s\n", &filename[0]);
	CHECK2(fd < 0);
	// mimic libusb_open
	int priv_size = usbi_backend->device_handle_priv_size;
	handle = malloc(sizeof(*handle) + priv_size);
	r = usbi_mutex_init(&handle->lock, NULL);
	CHECK();
	session_id = (busnum << 8) | (uint8_t) devnum;
	dev = usbi_alloc_device(libusb_ctx, session_id);
	handle->dev = libusb_ref_device(dev);
	handle->claimed_interfaces = 0;
	memset(&handle->os_priv, 0, priv_size);
	DINFO;
	// set fd
	struct linux_device_handle_priv *hpriv = (struct linux_device_handle_priv *) &handle->os_priv;
	hpriv->fd = fd;
	// finish mimicking
	list_add(&handle->list, &libusb_ctx->open_devs);
	usbi_fd_notification(libusb_ctx);
	DINFO;
	// return
	return 0;
}

int get_handle_using_libusb()
{
	DINFO;
	handle = libusb_open_device_with_vid_pid(libusb_ctx, VID, PID);
	DINFO;
	if (handle == NULL) {
		printf("No handle obtained. You may need to run this with sudo\n");
		exit(2);
	}
	return 0;
}

int proceed()
{
	int r;
	if (libusb_kernel_driver_active(handle, INTF)) {
		r = libusb_detach_kernel_driver(handle, INTF); // okay to fail b/c the driver may already have been detached
		CHECK();
	}
	r = libusb_claim_interface(handle, INTF);
	CHECK();
	r = libusb_set_interface_alt_setting(handle, INTF, ALTS);
	CHECK();
	struct libusb_transfer *tx1 = alloc_capture_transfer();
	r = libusb_submit_transfer(tx1);
	CHECK();
}

/* If args are given, they should be bus number and device number, expressed as strings */
int main(int argc, char * argv[])
{
	int r;
	// Get context
	r = libusb_init(&libusb_ctx);
	CHECK();
	if (argc > 2) {
		r = get_handle_using_fd(argv);
	} else {
		r = get_handle_using_libusb();
	}
	CHECK();
	proceed();
	CHECK();
}
