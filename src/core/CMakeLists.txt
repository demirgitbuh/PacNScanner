qt_add_library(pacn_core STATIC
    Types.h
    Result.h
    Logger.h            Logger.cpp
    NetUtils.h          NetUtils.cpp
    PlatformServices.h
    IpRange.h           IpRange.cpp
    Device.h            Device.cpp
    ScanConfig.h        ScanConfig.cpp
    PortProfiles.h      PortProfiles.cpp
    ServiceDetector.h   ServiceDetector.cpp
    VendorLookup.h      VendorLookup.cpp
    RiskEngine.h        RiskEngine.cpp
    OsFingerprinter.h   OsFingerprinter.cpp
    DeviceClassifier.h  DeviceClassifier.cpp
    HostnameResolver.h  HostnameResolver.cpp
    probe/Probe.h
    probe/TcpProbe.h    probe/TcpProbe.cpp
    probe/IcmpProbe.h   probe/IcmpProbe.cpp
    probe/DefaultProbeFactory.h probe/DefaultProbeFactory.cpp
    ScanEngine.h        ScanEngine.cpp
    ScanWorker.h        ScanWorker.cpp
)

target_link_libraries(pacn_core
    PUBLIC  pacn_config Qt6::Core Qt6::Network)

set_target_properties(pacn_core PROPERTIES AUTOMOC ON)
