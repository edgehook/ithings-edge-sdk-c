#!/bin/bash

SRC_ROOT=${PWD}
MQTT_INSTALL_PATH=${SRC_ROOT}/../

cd paho.mqtt.c/

# clean the make 
[ -f Makefile ] && make clean
# remove cmake cache.
rm -rf Makefile CMakeFiles CMakeCache.txt cmake_install.cmake CPackConfig.cmake CPackSourceConfig.cmake VersionInfo.h src/eclipse-paho-mqtt-cConfigVersion.cmake 

if [ "x$1" = "xclean" ];then
	echo "[clean Done]!"
	exit 0
fi

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

echo "[Done]..."
