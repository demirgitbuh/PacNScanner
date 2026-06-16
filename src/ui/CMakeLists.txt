qt_add_library(pacn_ui STATIC
    Palette.h
    ThemeManager.h          ThemeManager.cpp
    IconProvider.h          IconProvider.cpp
    StatCard.h              StatCard.cpp
    MiniCharts.h            MiniCharts.cpp
    NetworkInfoPanel.h      NetworkInfoPanel.cpp
    DeviceTableModel.h      DeviceTableModel.cpp
    DeviceFilterProxyModel.h DeviceFilterProxyModel.cpp
    DeviceDetailDialog.h    DeviceDetailDialog.cpp
    TopologyView.h          TopologyView.cpp
    LogPanel.h              LogPanel.cpp
    SettingsDialog.h        SettingsDialog.cpp
    AboutDialog.h           AboutDialog.cpp
    UpdateChecker.h         UpdateChecker.cpp
    MainWindow.h            MainWindow.cpp
)

target_link_libraries(pacn_ui
    PUBLIC  pacn_core pacn_platform pacn_storage pacn_reporting pacn_config
            Qt6::Widgets Qt6::Svg Qt6::Network)

set_target_properties(pacn_ui PROPERTIES AUTOMOC ON AUTOUIC ON)
