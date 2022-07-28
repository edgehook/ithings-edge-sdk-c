#!/bin/bash

SRC_ROOT=${PWD}
MQTT_INSTALL_PATH=$SRC_ROOT

cd 3rdparty/paho.mqtt.c/

# clean the make 
[ -f Makefile ] && make clean

# remove cmake cache.
rm -rf Makefile CMakeFiles CMakeCache.txt cmake_install.cmake CPackConfig.cmake CPackSourceConfig.cmake VersionInfo.h src/eclipse-paho-mqtt-cConfigVersion.cmake 

## clean 
echo $1
if [ "x$1" = "xclean" ];then
	rm -rf ${SRC_ROOT}/include/MQTT*.h
	echo "[clean Done]!" 
	exit 0
fi

# cmake
cmake . -DCMAKE_INSTALL_PREFIX=${MQTT_INSTALL_PATH}
if [ "$?" != 0 ];then
	echo "cmake failed"
	exit 1
fi
 
make && make install
if [ "$?" != 0 ];then
	echo "make or make intsall failed"
	exit 1
fi 
