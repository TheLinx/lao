all: build
	cd build; make

build:
	mkdir build
	cd build; cmake ..
