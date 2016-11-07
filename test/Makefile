LDIR := ../libusb
LIBH := $(LDIR)/libusb.h $(LDIR)/libusbi.h $(LDIR)/version.h $(LDIR)/os/linux_usbfs.h $(LDIR)/os/threads_posix.h
LIBC := $(LDIR)/core.c $(LDIR)/descriptor.c $(LDIR)/io.c $(LDIR)/sync.c $(LDIR)/os/linux_usbfs.c $(LDIR)/os/threads_posix.c

CLFAGS := -I.. -I$(LDIR) -I$(LDIR)/os \
	-DLIBUSB_DESCRIBE=\"1.0.9-28-g7634714\"

test: test.c $(LIBC) $(LIBH)
	gcc $(CLFAGS) -o test.out test.c $(LIBC) -lpthread
