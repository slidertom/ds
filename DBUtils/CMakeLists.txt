cmake_minimum_required(VERSION 3.9.0)

INCLUDE(ProjectOptions.cmake)

project(DBUtilsImpl)

set(DBUtils_Target DBUtilsImpl)

set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")

get_filename_component(DS_Root "${CMAKE_CURRENT_SOURCE_DIR}" PATH)

file(GLOB_RECURSE sources "*.cpp" "*.h" "*.rc" "*.c")
set(root ${CMAKE_CURRENT_SOURCE_DIR})
source_group(TREE ${root} FILES ${sources})
list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdCXX/CMakeCXXCompilerId.cpp")
list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdC/CMakeCCompilerId.c")
source_group("" FILES ${sources})

IF( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8" )
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoBinaryFieldUtil.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoCheckAddDBField.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoDatabaseImpl.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoDatabaseImpl.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoDatabaseUtil.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoErrorHandler.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoErrorHandler.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoErrorHandler.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DAOExtensions.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DAOExtensions.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoRecordsetImpl.h")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/Dao/DaoRecordsetImpl.cpp")
ENDIF()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LIBPATH:${DS_Root}/../Lib")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}/sqlite")

add_library	(${DBUtils_Target} SHARED
			${sources})
			
include_directories("${DS_Root}")

set_project_bin_lib_properties("../Lib")
set_project_definitions()
set_project_compile_flags()
set_git_branch_in_label()
set_dpi_mode()

IF( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8" )
    IF( ${WIN32} )
        list(APPEND COMPILEOPT
            /Gy			  	# Enable function level linking
            /Yu			  	# Use precompiled headers
            #/Zm800			# Precompiled header memory allocation limit. Disabled for #43854 - W40: /Zm800 # Precompiled header memory allocation limit => can be removed?
            /GR				# Enable run type information
            /Zc:twoPhase-   # C++20 requires this 
            /std:c++20 	
            #/std:c++latest #warning  #pragma warning (disable: 4857) // c1xx : warning C4857: C++/CLI mode does not support C++ versions newer than C++17; setting language to /std:c++17
        )
    ELSE()
        list(APPEND COMPILEOPT -std=c++20)
    ENDIF()
	target_compile_options(${PROJECT_NAME} PUBLIC  ${COMPILEOPT})
ELSE()
    IF( ${WIN32} )
        list(APPEND COMPILEOPT
            /Gy			  	# Enable function level linking
            /Yu			  	# Use precompiled headers
            #/Zm800			# Precompiled header memory allocation limit. Disabled for #43854 - W40: /Zm800 # Precompiled header memory allocation limit => can be removed?
            /GR				# Enable run type information
            /Zc:twoPhase-   # C++20 requires this 
            /std:c++17 	
            #/std:c++latest #warning  #pragma warning (disable: 4857) // c1xx : warning C4857: C++/CLI mode does not support C++ versions newer than C++17; setting language to /std:c++17
        )
    ELSE()
        list(APPEND COMPILEOPT -std=c++20)
    ENDIF()
	target_compile_options(${PROJECT_NAME} PUBLIC  ${COMPILEOPT})
ENDIF()

set_project_compile_options_debugrelease()
set_project_processor_definitions()
set_precompiled_header()

if (WIN32)
	set(adodotnetfile "${CMAKE_CURRENT_SOURCE_DIR}/AdoDotNet/AdoDotNetImpl.cpp")
	set_source_files_properties("${adodotnetfile}" PROPERTIES COMPILE_FLAGS "/Y- /clr /EHa /w44075")
endif (WIN32)

STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

# Visual Studio 2022 requires ManagedAssembly parameter set to true if CLR for one file is required
# Sample taken from https://gitlab.kitware.com/cmake/cmake/-/issues/22583
if(NOT CMAKE_GENERATOR MATCHES "Visual Studio 1[0-6]")
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_GLOBAL_ManagedAssembly "true")
endif()

if(NOT CMAKE_CL_64 AND WIN32)
  INCLUDE(mfcUse.cmake)
endif()

#set_source_non_precompiled_header("${CMAKE_CURRENT_SOURCE_DIR}/JsonUtils/yyjson/yyjson.c")
#set_source_non_precompiled_header("${CMAKE_CURRENT_SOURCE_DIR}/JsonUtils/yyjson/yyjson.h")
 
set_source_non_precompiled_header("${CMAKE_CURRENT_SOURCE_DIR}/JsonUtils/simdjson/simdjson.h")
set_source_non_precompiled_header("${CMAKE_CURRENT_SOURCE_DIR}/JsonUtils/simdjson/simdjson.cpp")
