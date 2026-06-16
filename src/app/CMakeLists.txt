qt_add_executable(pacnscanner WIN32 MACOSX_BUNDLE
    main.cpp
    ${CMAKE_SOURCE_DIR}/assets/resources.qrc
)

target_link_libraries(pacnscanner
    PRIVATE pacn_ui pacn_core pacn_platform pacn_storage pacn_reporting pacn_config
            Qt6::Widgets Qt6::Network)

set_target_properties(pacnscanner PROPERTIES
    OUTPUT_NAME "pacnscanner"
    MACOSX_BUNDLE_BUNDLE_NAME "PacNScanner"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.demirgitbuh.pacnscanner"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")

# Optional embedded translations.
if(Qt6LinguistTools_FOUND)
    qt_add_translations(pacnscanner
        TS_FILES
            ${CMAKE_SOURCE_DIR}/assets/i18n/pacnscanner_en.ts
            ${CMAKE_SOURCE_DIR}/assets/i18n/pacnscanner_tr.ts
        RESOURCE_PREFIX "/i18n")
endif()

install(TARGETS pacnscanner
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin)

# Bundle the Qt runtime on install so CPack archives/installers are standalone
# (runs windeployqt / macdeployqt under the hood). No-op on Linux.
if(COMMAND qt_generate_deploy_app_script AND (WIN32 OR APPLE))
    qt_generate_deploy_app_script(
        TARGET pacnscanner
        OUTPUT_SCRIPT pacn_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${pacn_deploy_script})
endif()

if(UNIX AND NOT APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/pacnscanner.desktop
            DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg
            DESTINATION share/icons/hicolor/scalable/apps
            RENAME pacnscanner.svg)
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/io.github.demirgitbuh.PacNScanner.metainfo.xml
            DESTINATION share/metainfo)
endif()
