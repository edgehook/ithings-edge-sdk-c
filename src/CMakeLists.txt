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

IF (WIN32)
    SET(LIBS_SYSTEM ws2_32 crypt32 RpcRT4)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
ELSEIF (UNIX)
	IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
        SET(LIBS_SYSTEM c dl pthread rt)
        # anl is only available with glibc so check if it is found before using
        # it or build will fail on uclibc or musl
        FIND_LIBRARY(LIB_ANL anl)
        IF(LIB_ANL)
            SET(LIBS_SYSTEM "${LIBS_SYSTEM}" anl)
        ENDIF()
        ADD_DEFINITIONS(-D_GNU_SOURCE -fvisibility=hidden)
    ELSE()
        SET(LIBS_SYSTEM c pthread)
    ENDIF()
ENDIF()

IF (NOT WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror ")
ENDIF()

## find mqtt library.
SET(CMAKE_INSTALL_LIBDIR  "${CMAKE_INSTALL_PREFIX}/lib")
SET(PAHO_MQTT_SEARCH_PATH  "${PROJECT_SOURCE_DIR}/lib")

FIND_LIBRARY(PAHO_MQTT3a_LIB NAMES paho-mqtt3a paho-mqtt3c
		HINTS ${PAHO_MQTT_SEARCH_PATH} CMAKE_FIND_ROOT_PATH_BOTH)
FIND_LIBRARY(PAHO_MQTT3c_LIB NAMES paho-mqtt3c
		HINTS ${PAHO_MQTT_SEARCH_PATH} CMAKE_FIND_ROOT_PATH_BOTH)
MESSAGE(STATUS "paho mqtt hints: ${PAHO_MQTT3a_LIB}")
MESSAGE(STATUS "paho mqtt hints: ${PAHO_MQTT3c_LIB}")

ADD_LIBRARY(mapper-core SHARED ${util_src} ${transport_src} ${core_src})
TARGET_LINK_LIBRARIES(mapper-core ${LIBS_SYSTEM} ${PAHO_MQTT3a_LIB} ${PAHO_MQTT3c_LIB})
SET_TARGET_PROPERTIES(
        mapper-core PROPERTIES
        VERSION "1.0.0"
        SOVERSION "1.0.0"
        COMPILE_DEFINITIONS "MAPPER_CORE_EXPORTS=1")

TARGET_INCLUDE_DIRECTORIES(mapper-core
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  PRIVATE
    ${CMAKE_BINARY_DIR})

INSTALL(TARGETS mapper-core
		EXPORT mapper-core-cTargets
        LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR})

INSTALL(EXPORT mapper-core-cTargets
    FILE mapper-core-cConfig.cmake
    NAMESPACE mapper-core-c::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mapper-core-c)
