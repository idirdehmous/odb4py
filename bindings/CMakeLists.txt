#
#  odb4py
#  Copyright (C) 2026 Royal Meteorological Institute of Belgium (RMI)
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
# CMakeLists.txt : Compile the C/Python  extension 
# Compiles in linux env or a Docker container



# Dependencies 
find_package(SQLite3  )
find_package( NetCDF4 COMPONENTS ${NETCDF_FIND_COMPONENTS} )
find_package(ecbuild )
find_package(eckit   )
find_package(odc     )
list(APPEND CMAKE_PREFIX_PATH "/usr/local")

set ( NCLIB       "/usr/lib")  
set ( SQLITE_LIB  "/usr/local/lib")

list(APPEND CMAKE_PREFIX_PATH  ${NCLIB} )
list(APPEND CMAKE_PREFIX_PATH  ${SQLITE_LIB} )
list(APPEND CMAKE_PREFIX_PATH  ${ECBUILD_LIB})
list(APPEND CMAKE_PREFIX_PATH "/usr/local")


execute_process( COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NumPy_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE )

message(STATUS "NumPy include dir: ${NumPy_INCLUDE_DIR}")


# Netcdf  
set (NETCDF_C_INCLUDE_DIRS  ${NETCDF_INCLUDE_DIR})
set (NETCDF_C_LIBRARIES     ${NETCDF_LIBRARY}    )

# eckit , odc 
set (ODC_INCLUDE   "/usr/local/include")
set (ECKIT_INCLUDE "/usr/local/include")



# Create the  odb4py  C extension   
add_library(   core     MODULE  core.c     )
add_library(   convert  MODULE  convert.c  )
add_library(   info     MODULE  info.c     )

target_include_directories(core  PRIVATE      ${Python3_INCLUDE_DIRS}   ${NumPy_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/include  ${SQLite3_INCLUDE_DIRS})
target_include_directories(info  PRIVATE      ${Python3_INCLUDE_DIRS}   ${NumPy_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/include  ${SQLite3_INCLUDE_DIRS})

# convert.so needs more libs and include
target_include_directories(convert PRIVATE    ${Python3_INCLUDE_DIRS}  
                                              ${NumPy_INCLUDE_DIR}  
					      ${CMAKE_SOURCE_DIR}/include  
					      ${SQLite3_INCLUDE_DIRS}
					      ${ODC_INCLUDE}
					      ${ECKIT_INCLUDE} 
					      )



target_link_libraries(core       PRIVATE Python3::Module   odb_lib ${Python3_LIBRARIES})
target_link_libraries(info       PRIVATE Python3::Module   odb_lib ${Python3_LIBRARIES})
target_link_libraries(convert    PRIVATE Python3::Module sqlite3 
	                                                 netcdf  
							 odb_lib 
							 odccore 
							 eckit							 
							 ${NETCDF_C_INCLUDE_DIRS}  
							 ${NETCDF_C_LIBRARIES} 
							 ${Python3_LIBRARIES}  
							 ${SQLite3_LIBRARIES} 
							 #							 ${ODC_LIB}
							 #${ECKIT_LIB} 
							 )


# Module output name  (.so) to be imported in python 
set_target_properties(core    PROPERTIES OUTPUT_NAME  "core"    PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")
set_target_properties(info    PROPERTIES OUTPUT_NAME  "info"    PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")
set_target_properties(convert PROPERTIES OUTPUT_NAME  "convert" PREFIX ""   BUILD_RPATH "\$ORIGIN/lib"  INSTALL_RPATH "\$ORIGIN/lib")


# Install  the modules
install( TARGETS core  LIBRARY DESTINATION     . )
install( TARGETS info   LIBRARY DESTINATION    . )
install( TARGETS convert  LIBRARY DESTINATION  . )

# pure python module + __init__.py 
install( DIRECTORY ${CMAKE_SOURCE_DIR}/utils/    DESTINATION utils   FILES_MATCHING PATTERN "*.py" )

