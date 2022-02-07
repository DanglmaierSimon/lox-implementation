if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/cpplox CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package cpplox)

install(
    TARGETS cpplox
    EXPORT cpploxTargets
    RUNTIME COMPONENT cpplox_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    cpplox_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(cpplox_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${cpplox_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT cpplox_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${cpplox_INSTALL_CMAKEDIR}"
    COMPONENT cpplox_Development
)

install(
    EXPORT cpploxTargets
    NAMESPACE cpplox::
    DESTINATION "${cpplox_INSTALL_CMAKEDIR}"
    COMPONENT cpplox_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
