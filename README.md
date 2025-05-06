# Syengine

## Description
This is a game engine that I am making to learn xcb, vulkan, ecs pattern, and other technologies.

## Building

	sudo pacman -S vulkan-devel cmake base-devel
	echo export VULKAN_SDK=/usr >> ~/.bashrc
	source ~/.bashrc

    git clone https://github.com/RyanDKennedy/syengine.git
    cd syengine
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .

	../bin/release/syengine

# experimental script for ftdi stuff

su
groupadd usb
echo 'SUBSYSTEMS=="usb", ACTION=="add", MODE="0664", GROUP="usb"' >> /etc/udev/rules.d/99-usbftdi.rules
/etc/init.d/udev reload
echo 'blacklist ftdi_sio' > /etc/modprobe.d/ftdi.conf
rmmod ftdi_sio
usermod -aG usb (your username here)
exit
reboot
