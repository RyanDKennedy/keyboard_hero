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


