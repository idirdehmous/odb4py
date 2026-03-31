# CMakeLists.txt : Compile the C/Python  extension 


find_package(SQLite3)

set ( NCDIR  "/usr/lib"  )  
set ( SQLITE_DIR  "/usr/local/lib" )

list(APPEND CMAKE_PREFIX_PATH  ${NCDIR} )
list(APPEND CMAKE_PREFIX_PATH  ${SQLITE_DIR} )

find_package( NetCDF4 COMPONENTS ${NETCDF_FIND_COMPONENTS} )

execute_process( COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NumPy_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE )

message(STATUS "NumPy include dir: ${NumPy_INCLUDE_DIR}")


# Netcdf  
set (NETCDF_C_INCLUDE_DIRS  ${NETCDF_INCLUDE_DIR})
set (NETCDF_C_LIBRARIES     ${NETCDF_LIBRARY}    )

# SQLite 
#SQLite3_INCLUDE_DIRS
#SQLite3_LIBRARIES


# Create the  odb4py  C extension   
add_library(   core     MODULE  core.c     )
add_library(   convert  MODULE  convert.c  )
add_library(   info     MODULE  info.c     )

target_include_directories(core  PRIVATE      ${Python3_INCLUDE_DIRS}   ${NumPy_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/include  ${SQLite3_INCLUDE_DIRS})
target_include_directories(convert PRIVATE    ${Python3_INCLUDE_DIRS}   ${NumPy_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/include  ${SQLite3_INCLUDE_DIRS})
target_include_directories(info  PRIVATE      ${Python3_INCLUDE_DIRS}   ${NumPy_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/include  ${SQLite3_INCLUDE_DIRS})

target_link_libraries(core       PRIVATE Python3::Module   odb_lib ${Python3_LIBRARIES})
target_link_libraries(info       PRIVATE Python3::Module   odb_lib ${Python3_LIBRARIES})
target_link_libraries(convert    PRIVATE Python3::Module sqlite3 netcdf  odb_lib  ${NETCDF_C_INCLUDE_DIRS}  ${NETCDF_C_LIBRARIES} ${Python3_LIBRARIES}  ${SQLite3_LIBRARIES})


# Module output name  (.so) to be imported in python 
set_target_properties(core    PROPERTIES OUTPUT_NAME  "core"    PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")
set_target_properties(convert PROPERTIES OUTPUT_NAME  "convert" PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")
set_target_properties(info    PROPERTIES OUTPUT_NAME  "info"    PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")

# Install  the module and the other pure python modules 
install( TARGETS core  LIBRARY DESTINATION  . )
install( TARGETS convert  LIBRARY DESTINATION  . )
install( TARGETS info   LIBRARY DESTINATION  . )

install( DIRECTORY ${CMAKE_SOURCE_DIR}/utils/    DESTINATION utils   FILES_MATCHING PATTERN "*.py" )

