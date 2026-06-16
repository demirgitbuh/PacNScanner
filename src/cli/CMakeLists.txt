qt_add_executable(pacnscanner-cli
    main.cpp
    ${CMAKE_SOURCE_DIR}/assets/resources.qrc
)

target_link_libraries(pacnscanner-cli
    PRIVATE pacn_core pacn_platform pacn_reporting pacn_config
            Qt6::Core Qt6::Network)

set_target_properties(pacnscanner-cli PROPERTIES OUTPUT_NAME "pacnscanner-cli")

install(TARGETS pacnscanner-cli RUNTIME DESTINATION bin)
