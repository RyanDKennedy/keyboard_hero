# Keyboard Hero

## Description
This is a guitar hero like game except using a custom embedded keyboard device. Only works on linux systems running xorg. Tested on arch linux, and ubuntu 22.04

## Controls

### Main Menu
| Key | Action |
| --- | --- |
| arrow up | up |
| arrow down | down |
| arrow right | select |

### Map Picker
| Key | Action |
| --- | --- |
| arrow up | up |
| arrow down | down |
| enter | select |

### Play
| Key | Action |
| --- | --- |
| 1 | first key |
| 2 | second key |
| 3 | third key |
| 4 | fourth key |

### Score Display
| Key | Action |
| --- | --- |
| escape | go to main menu |

### Map Creator

| Key | Action |
| --- | --- |
| e | place note |
| q | delete selected note |
| d | select next note |
| a | select previous note |
| o | shrink song length |
| p | grow song length |
| space | grow note duration |
| left shift | shrink note duration |
| arrow left | move note a key left |
| arrow right | move note a key right |
| arrow up | move note forward |
| arrow down | move note backwards |
| w | move player forwards |
| s | move player backwards |

## Building

Install ftd2xx library - https://ftdichip.com/drivers/d2xx-drivers/

	sudo pacman -S vulkan-devel cmake base-devel sqlite3 openal freealut libxcb libxkbcommon libxkbcommon-x11 libxi
	echo export VULKAN_SDK=/usr >> ~/.bashrc
	source ~/.bashrc

    git clone https://github.com/RyanDKennedy/keyboard_hero.git
    cd keyboard_hero
    mkdir build
    cd build
    cmake ..
    cmake --build . -j

	../bin/release/syengine

# experimental script for keyboard device

Run these commands if you want to be able to use the custom keyboard embedded device when running as user.

	su
	groupadd usb
	echo 'SUBSYSTEMS=="usb", ACTION=="add", MODE="0664", GROUP="usb"' >> /etc/udev/rules.d/99-usbftdi.rules
	/etc/init.d/udev reload
	echo 'blacklist ftdi_sio' > /etc/modprobe.d/ftdi.conf
	rmmod ftdi_sio
	usermod -aG usb (your username here)
	exit
	reboot
