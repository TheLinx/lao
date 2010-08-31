all: build
	cd build; make

build:
	mkdir build
	cd build; cmake ..

test: all
	lua -i -e 'package.cpath = "./build/lib?.so" require"ao"'
