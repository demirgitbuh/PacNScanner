set(PACN_PLATFORM_SOURCES
    PlatformFactory.cpp
    AdapterUtil.h          AdapterUtil.cpp
    fake/FakePlatform.h    fake/FakePlatform.cpp
    generic/GenericUnixPlatform.h generic/GenericUnixPlatform.cpp
)

if(WIN32)
    list(APPEND PACN_PLATFORM_SOURCES
        windows/WindowsPlatform.h windows/WindowsPlatform.cpp)
endif()

qt_add_library(pacn_platform STATIC ${PACN_PLATFORM_SOURCES})

target_link_libraries(pacn_platform
    PUBLIC  pacn_core pacn_config
    PRIVATE Qt6::Core Qt6::Network)

if(WIN32)
    target_link_libraries(pacn_platform PRIVATE iphlpapi ws2_32)
endif()
