sudo mount /dev/sdb1 /mnt/rpi-myos-onsd/
sudo rm -f /mnt/rpi-myos-onsd/kernel8.img
sudo cp kernel8.img /mnt/rpi-myos-onsd
sudo umount /mnt/rpi-myos-onsd
echo Done!