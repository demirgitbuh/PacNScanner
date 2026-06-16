# FindNpcap.cmake
# Locates the optional Npcap SDK (Windows). When found, defines an imported
# target Npcap::Npcap. When not found, raw/ARP-active scanning is silently
# disabled and PacNScanner falls back to QtNetwork-only discovery.
#
# Hints:
#   NPCAP_ROOT  - root of the Npcap SDK (contains Include/ and Lib/)

if(NOT WIN32)
  return()
endif()

find_path(NPCAP_INCLUDE_DIR
  NAMES pcap.h
  HINTS "${NPCAP_ROOT}" "$ENV{NPCAP_ROOT}"
  PATH_SUFFIXES Include include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_npcap_lib_suffixes "Lib/x64" "lib/x64")
else()
  set(_npcap_lib_suffixes "Lib" "lib")
endif()

find_library(NPCAP_PACKET_LIBRARY
  NAMES Packet packet
  HINTS "${NPCAP_ROOT}" "$ENV{NPCAP_ROOT}"
  PATH_SUFFIXES ${_npcap_lib_suffixes})

find_library(NPCAP_WPCAP_LIBRARY
  NAMES wpcap
  HINTS "${NPCAP_ROOT}" "$ENV{NPCAP_ROOT}"
  PATH_SUFFIXES ${_npcap_lib_suffixes})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Npcap
  REQUIRED_VARS NPCAP_INCLUDE_DIR NPCAP_PACKET_LIBRARY NPCAP_WPCAP_LIBRARY)

if(Npcap_FOUND AND NOT TARGET Npcap::Npcap)
  add_library(Npcap::Npcap UNKNOWN IMPORTED)
  set_target_properties(Npcap::Npcap PROPERTIES
    IMPORTED_LOCATION "${NPCAP_WPCAP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NPCAP_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${NPCAP_PACKET_LIBRARY}")
endif()

mark_as_advanced(NPCAP_INCLUDE_DIR NPCAP_PACKET_LIBRARY NPCAP_WPCAP_LIBRARY)
