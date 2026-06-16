qt_add_library(pacn_storage STATIC
    Database.h       Database.cpp
    SettingsStore.h  SettingsStore.cpp
)

target_link_libraries(pacn_storage
    PUBLIC  pacn_core pacn_config Qt6::Core Qt6::Sql)

set_target_properties(pacn_storage PROPERTIES AUTOMOC ON)
