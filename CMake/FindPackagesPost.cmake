
if(WIN32)
  list(APPEND COMMON_PACKAGE_DEFINES WIN32 WIN32_API WIN32_LEAN_AND_MEAN)
endif(WIN32)

if(UDT_FOUND AND NOT UDT_HAS_RCVDATA)
  message(STATUS "Disable old UDT version, missing UDT_RCVDATA")
  set(UDT_FOUND)
endif()

if(LUNCHBOX_USE_DNSSD)
  set(FIND_PACKAGES_FOUND "${FIND_PACKAGES_FOUND} zeroconf")
  list(APPEND COMMON_PACKAGE_DEFINES COLLAGE_USE_SERVUS)
endif()

if(COLLAGE_AGGRESSIVE_CACHING)
  list(APPEND COMMON_PACKAGE_DEFINES COLLAGE_AGGRESSIVE_CACHING)
endif()

if(COMMON_BIGENDIAN)
  list(APPEND COMMON_PACKAGE_DEFINES COMMON_BIGENDIAN)
endif()

if(COLLAGE_BUILD_V2_API)
  list(APPEND COMMON_PACKAGE_DEFINES COLLAGE_V2_API)
else()
  list(APPEND COMMON_PACKAGE_DEFINES COLLAGE_V1_API)
endif()
