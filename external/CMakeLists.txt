#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS external tools.
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current tools include:
#    UnicodeData.txt from the Unicode Consortium (http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt)
#

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

#
# Versions of externals to download
#
set(ZLIB_VERSION "1.2.11")
set(ZSTD_VERSION "1.4.4")


#
# Get Zlib (zlib)
#

set(ZLIB_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib")
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib/src/zlib" PARENT_SCOPE)
set(ZLIB_INCLUDE_DIR_2 "${CMAKE_CURRENT_SOURCE_DIR}/zlib" PARENT_SCOPE)

if (MSVC)
	set(ZLIB_GENERATOR "NMake Makefiles")
	set(ZLIB_STATIC_LIB "${ZLIB_BUILD_DIR}/zlibstaticd.lib" PARENT_SCOPE)
else()
	set(ZLIB_GENERATOR "Unix Makefiles")
	set(ZLIB_STATIC_LIB "${ZLIB_BUILD_DIR}/libz.a" PARENT_SCOPE)
endif()

ExternalProject_Add(zlib
	URL "https://github.com/madler/zlib/archive/v${ZLIB_VERSION}.tar.gz"
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/zlib
	UPDATE_COMMAND ""
	INSTALL_COMMAND ""
	BINARY_DIR ${ZLIB_BUILD_DIR}
	CONFIGURE_COMMAND cmake -G ${ZLIB_GENERATOR} ${ZLIB_BUILD_DIR}/src/zlib
	BUILD_COMMAND $(MAKE)
	BUILD_BYPRODUCTS ${ZLIB_STATIC_LIB}
)

#
# Get ZStandard (zstd)
#

set(ZSTD_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zstd")
set(ZSTD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zstd/src/zstd/lib" PARENT_SCOPE)

if (MSVC)
	set (ZSTD_GENERATOR "NMake Makefiles")
	set(ZSTD_STATIC_LIB "${ZSTD_BUILD_DIR}/lib/zstd_static.lib" PARENT_SCOPE)
else()
	set (ZSTD_GENERATOR "Unix Makefiles")
	set(ZSTD_STATIC_LIB "${ZSTD_BUILD_DIR}/lib/libzstd.a" PARENT_SCOPE)
endif()

ExternalProject_Add(zstd
	URL "https://github.com/facebook/zstd/archive/v${ZSTD_VERSION}.tar.gz"
	PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/zstd
	UPDATE_COMMAND ""
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND cmake -G ${ZSTD_GENERATOR} ${ZSTD_BUILD_DIR}/src/zstd/build/cmake
	CMAKE_ARGS -DZSTD_BUILD_STATIC=ON
	BINARY_DIR ${ZSTD_BUILD_DIR}
	BUILD_COMMAND $(MAKE) libzstd_static
	BUILD_BYPRODUCTS ${ZSTD_STATIC_LIB}
)

add_custom_target(refresh_externals)

#
# get the latest version of the Unicode data file "UnicodeData.txt" from the Unicode web site
#

add_custom_command(
	TARGET refresh_externals PRE_BUILD
	COMMAND curl http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt > UnicodeData.txt
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Unicode
	)

add_custom_command(
	TARGET refresh_externals PRE_BUILD
	COMMAND curl http://www.unicode.org/Public/UCD/latest/ucd/PropList.txt > PropList.txt
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Unicode
	)

