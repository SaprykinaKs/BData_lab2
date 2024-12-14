cmake_minimum_required(VERSION 3.10)
project(WxDatabaseApp)

set(CMAKE_CXX_STANDARD 17)

# set(wxWidgets_ROOT_DIR /opt/homebrew/Cellar/wxwidgets/3.2.6)
# set(wxWidgets_LIB_DIR /opt/homebrew/Cellar/wxwidgets/3.2.6/lib)

# find_package(wxWidgets REQUIRED COMPONENTS core base)
# include(${wxWidgets_USE_FILE})

# find_library(XLSXWRITER_LIBRARY NAMES xlsxwriter PATHS /opt/homebrew/Cellar/libxlsxwriter/1.1.9/lib REQUIRED)
# include_directories(/opt/homebrew/Cellar/libxlsxwriter/1.1.9/include)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/wxwidgets/3.2.6/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/wxwidgets/3.2.6/lib)

find_library(XLSXWRITER_LIBRARY NAMES xlsxwriter PATHS ${CMAKE_CURRENT_SOURCE_DIR}/deps/libxlsxwriter/1.1.9/lib REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/libxlsxwriter/1.1.9/include)

set(SOURCES
    src/main.cpp
    src/Database.cpp
)

add_executable(WxDatabaseApp ${SOURCES})
target_link_libraries(WxDatabaseApp ${wxWidgets_LIBRARIES} ${XLSXWRITER_LIBRARY} -lstdc++)
