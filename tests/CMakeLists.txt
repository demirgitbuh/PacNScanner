include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Core / integration tests ------------------------------------------------
qt_add_executable(pacn_core_tests
    test_main.cpp
    fake/FakeProbeFactory.h
    test_iprange.cpp
    test_netutils.cpp
    test_portprofiles.cpp
    test_servicedetector.cpp
    test_vendorlookup.cpp
    test_riskengine.cpp
    test_deviceclassifier.cpp
    test_reporting.cpp
    test_scanengine.cpp
    ${CMAKE_SOURCE_DIR}/assets/resources.qrc
)
target_link_libraries(pacn_core_tests
    PRIVATE pacn_core pacn_platform pacn_reporting pacn_config
            GTest::gtest Qt6::Core Qt6::Network)
add_test(NAME core_tests COMMAND pacn_core_tests)

# --- UI smoke test -----------------------------------------------------------
if(PACN_HAVE_GUI)
    qt_add_executable(pacn_ui_tests
        test_ui_smoke.cpp
        ${CMAKE_SOURCE_DIR}/assets/resources.qrc)
    target_link_libraries(pacn_ui_tests
        PRIVATE pacn_ui pacn_config GTest::gtest Qt6::Widgets)
    add_test(NAME ui_smoke COMMAND pacn_ui_tests)
    set_tests_properties(ui_smoke PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
