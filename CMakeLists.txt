cmake_minimum_required (VERSION 2.6)
project (mcp3425)
set (mcp3425_VERSION_MAJOR 0)
set (mcp3425_VERSION_MINOR 1)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

add_executable(test1 test1.c)
