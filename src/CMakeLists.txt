INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

SET(util_src
	util/cJSON.c
	util/cJSON_Utils.c
	util/blockedqueue.c
	util/list.c
	util/log.c
	util/json_util.c
	util/event_listener.c 
	util/string_util.c
	util/thread.c
	util/thread_pool.c
	util/util.c
	util/uuid.c
	)

SET(transport_src
	transport/message.c
	transport/mqtt.c
	transport/mqtt.h
	transport/transport.c
	)

SET(core_src
	common.c
	device.c
	mapper_core.c
	)

SET(LIBS_SYSTEM c dl pthread rt)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror ")
## find mqtt library.
SET(CMAKE_INSTALL_LIBDIR  "${CMAKE_INSTALL_PREFIX}/lib")
SET(PAHO_MQTT_SEARCH_PATH  "${PROJECT_SOURCE_DIR}/lib")

FIND_LIBRARY(PAHO_MQTT3a_LIB NAMES paho-mqtt3a paho-mqtt3c
		HINTS ${PAHO_MQTT_SEARCH_PATH})
FIND_LIBRARY(PAHO_MQTT3c_LIB NAMES paho-mqtt3c
		HINTS ${PAHO_MQTT_SEARCH_PATH})
MESSAGE(STATUS "paho mqtt hints: ${PAHO_MQTT3a_LIB}")
MESSAGE(STATUS "paho mqtt hints: ${PAHO_MQTT3c_LIB}")

ADD_LIBRARY(mapper-core SHARED ${util_src} ${transport_src} ${core_src})
TARGET_LINK_LIBRARIES(mapper-core ${LIBS_SYSTEM} ${PAHO_MQTT3a_LIB} ${PAHO_MQTT3c_LIB})
INSTALL(TARGETS mapper-core
        ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR})
