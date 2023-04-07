SDCARDFS_VERSION="0.1"

EXTRA_CFLAGS += -DSDCARDFS_VERSION=\"$(SDCARDFS_VERSION)\"

obj-$(CONFIG_SDCARD_FS) += sdcardfs.o

sdcardfs-y := dentry.o file.o inode.o main.o super.o lookup.o mmap.o packagelist.o derived_perm.o
#ifdef VENDOR_EDIT
sdcardfs-y += dev.o
sdcardfs-y += xattr.o
sdcardfs-y += dellog.o
#endif /* VENDOR_EDIT */
