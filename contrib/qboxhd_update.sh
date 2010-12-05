#!/bin/sh

search_dev ()
{
	if [ "$1" == "/dev/sda" ]; then
		SDA=1
	elif [ "$1" == "/dev/sdb" ]; then
		SDB=1
	else 
		SDX=$(( SDX + 1 ))
	fi
}

echo " "
echo "****************************"
echo "    CrossEPG Software"
echo "****************************"

SDA=0
SDB=0
SDX=0


DEVS=$(ls /sys/block/sd* | awk -F: '/\/sys\/block/ { print $1 }')
echo "DEVS: $DEVS"

for i in $DEVS
do
    sd_dev=$(echo "$i" | awk '/\/sys\/block\/sd[a-z]/')
	if [ "$sd_dev" != "" ]; then
		echo "CrossEPG: Device detected: $i"
		sd_dev_ch=$(echo "$sd_dev" | sed 's/\(\/sys\/block\/\)\(sd[a-z]\)/\/dev\/\2/')
		search_dev $sd_dev_ch
	fi
done



TOTAL_DEVS=$((SDA + SDB + SDX ))



if [ "$TOTAL_DEVS" -ne "2" -o "$SDA" -ne "1" -o "$SDB" -ne "1" ]; then
	echo " "
	echo "****************************"
	echo "FATAL: There must be only two USB flash drives,"
	echo "       the recovery drive and the corrupted drive"
	echo "       Devices found: sda: $SDA, sdb: $SDB, unknown: $SDX"
	exit
fi

MOUNTS=$(mount | awk -F' ' '/mnt\/update_fs/ { print $1 }')

if [ "$MOUNTS" == "/dev/sda1" -o "$MOUNTS" == "/dev/sda" ]; then
	BACKUP_DEV="sda1"
	ROOTFS_DEV="sdb1"
else
	BACKUP_DEV="sdb1"
	ROOTFS_DEV="sda1"
fi

echo "CrossEPG: Mounting rootfs"
mount /dev/$ROOTFS_DEV /mnt/new_root

echo "CrossEPG: Installing ..."

DST_DIR="/mnt/new_root"
SRC_DIR="/mnt/update_fs/qboxhd_update"

mkdir $DST_DIR/var/crossepg

# Copy CrossEPG python plugin .... #
cp -af $SRC_DIR/var/crossepg $DST_DIR/var/
cp -af $SRC_DIR/var/addons/uninstall $DST_DIR/var/addons/

cp -af $SRC_DIR/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG $DST_DIR/usr/local/lib/enigma2/python/Plugins/SystemPlugins/
cp -af $SRC_DIR/usr/local/lib/python2.6/* $DST_DIR/usr/local/lib/python2.6


# Permissions...#
chmod 755 $DST_DIR/var/crossepg/*
chmod 755 $DST_DIR/var/addons/uninstall/*
chmod 755 $DST_DIR/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/*.py

# link to python interpreter (usually in /usr/bin/)
cd $DST_DIR/usr/bin
ln -sf ../local/bin/python python
cd /

# Rename the dir qboxhd_update to done_qboxhd_update
mv /mnt/update_fs/qboxhd_update /mnt/update_fs/done_qboxhd_update

echo "CrossEPG: Unmounting rootfs"
umount /mnt/new_root

echo "CrossEPG: Done!"

