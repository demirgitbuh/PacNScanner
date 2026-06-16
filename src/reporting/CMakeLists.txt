qt_add_library(pacn_reporting STATIC
    Exporter.h
    Exporters.cpp
    HtmlReport.cpp
    Importers.cpp
)

target_link_libraries(pacn_reporting
    PUBLIC  pacn_core pacn_config Qt6::Core)
