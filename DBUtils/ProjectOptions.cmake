set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_SUPPRESS_REGENERATION TRUE)
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "")

get_filename_component(RootDir "${CMAKE_CURRENT_LIST_DIR}" PATH)
get_filename_component(RootDir "${RootDir}" PATH)

function(include_dir relativePath)
	include_directories("${RootDir}/${relativePath}")
endfunction(include_dir)

function(link_dir relativePath)
	link_directories("${RootDir}/${relativePath}")
endfunction(link_dir)

function(include_root_dir)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
endfunction(include_root_dir)

macro(set_project_dll_mode LibDirPath)
	file(GLOB_RECURSE sources "*.cpp" "*.h" "*.c" "*.rc" "*.bmp" "*.cur" "*ico")
	set(root ${CMAKE_CURRENT_SOURCE_DIR})
	source_group(TREE ${root} FILES ${sources})
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdCXX/CMakeCXXCompilerId.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdC/CMakeCCompilerId.c")
	source_group("" FILES ${sources})
	add_library	(${PROJECT_NAME} SHARED ${sources})
	include_root_dir()
	set_project_bin_lib_properties("${LibDirPath}")
	set_project_compile_options()
	set_project_definitions()
	set_project_compile_flags()
	set_project_processor_definitions()
	set_precompiled_header()
	set_git_branch_in_label()
	set_dpi_mode()
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endmacro(set_project_dll_mode)

macro(set_project_lib_mode)
	file(GLOB_RECURSE sources "*.cpp" "*.h" "*.c" "*.rc" "*.bmp" "*.cur" "*ico")
	set(root ${CMAKE_CURRENT_SOURCE_DIR})
	source_group(TREE ${root} FILES ${sources})
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdCXX/CMakeCXXCompilerId.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdC/CMakeCCompilerId.c")
	source_group("" FILES ${sources})
	add_library	(${PROJECT_NAME} ${sources})
	include_root_dir()
	set_project_definitions()
	set_project_compile_flags()
	set_project_processor_definitions()
	set_git_branch_in_label()
	set_dpi_mode()
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endmacro(set_project_lib_mode)

macro(set_project_exe_mode)
	file(GLOB_RECURSE sources "*.cpp" "*.h" "*.c" "*.rc" "*.bmp" "*.cur" "*ico")
	set(root ${CMAKE_CURRENT_SOURCE_DIR})
	source_group(TREE ${root} FILES ${sources})
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdCXX/CMakeCXXCompilerId.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdC/CMakeCCompilerId.c")
	source_group("" FILES ${sources})
	add_executable(${PROJECT_NAME} ${sources})
	include_root_dir()
	set_project_compile_options()
	set_project_definitions()
	set_project_compile_flags()
	set_project_processor_definitions()
	set_precompiled_header()
	set_git_branch_in_label()
	set_dpi_mode()
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endmacro(set_project_exe_mode)

macro(set_project_console_mode)
	file(GLOB_RECURSE sources "*.cpp" "*.h" "*.c" "*.rc" "*.bmp" "*.cur" "*ico")
	set(root ${CMAKE_CURRENT_SOURCE_DIR})
	source_group(TREE ${root} FILES ${sources})
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdCXX/CMakeCXXCompilerId.cpp")
	list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${CMAKE_VERSION}/CompilerIdC/CMakeCCompilerId.c")
	source_group("" FILES ${sources})
	add_executable(${PROJECT_NAME} ${sources})
    include_root_dir()
	set_project_compile_options()
	set_project_definitions()
	set_console_project_compile_flags()
	set_project_processor_definitions()
	set_precompiled_header()
	set_git_branch_in_label()
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endmacro(set_project_console_mode)

function(set_git_branch_in_label)
    IF (NOT WIN32) 
        return()
    ENDIF()

    find_package(Git)

	IF( !GIT_FOUND )
	   message(STATUS "Git executable was not found, check if you have Git installed.")
	   return()
	ENDIF()

	IF(NOT EXISTS "${RootDir}/.git")
		message(STATUS "Warning: .git folder does not exist.")
		return()
	ENDIF()

	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
		RESULT_VARIABLE result
		OUTPUT_VARIABLE output
	)
	
	STRING(REPLACE "\n" "" branch_name ${output})
	
	IF( NOT "${branch_name}" STREQUAL "master" )
		set_target_properties(${PROJECT_NAME} PROPERTIES 
								PROJECT_LABEL "${PROJECT_NAME}(${branch_name})")
	ENDIF()
endfunction(set_git_branch_in_label)

function(set_precompiled_header)
		set(FILENAME "stdafx.h")
		set(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp")
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/Yu\"${FILENAME}\" /DUSE_PRECOMPILED_HEADERS")
        set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "/Yc")
endfunction(set_precompiled_header)

function(set_dpi_mode)
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "OFF")
endfunction(set_dpi_mode)

function(set_source_non_precompiled_header filename)
		set_source_files_properties("${filename}" PROPERTIES COMPILE_FLAGS "/Y-")
endfunction(set_source_non_precompiled_header)

# (Generate Debug Info)
# Michael Taylor - 4/18/07
# The nice thing about having PDB files is when a crash occurs (in release or debug mode) you can get a usable callstack that can be debugged.  
# Without the PDB file you are stuck with the raw memory address of the line that crashed.  From here you'd have to use a map file, 
# generated during compilation, that mapped raw addresses to methods.  You'd also have to take the actual load address of the binary into 
# account and adjust accordingly.  With a PDB available programs like Doc Watson can report on the actual method that caused the exception 
# rather than just the memory address.  This makes debugging release mode/customer issues a lot easier.
# 
# Should you ship and install them?  That depends.  I have, for years, recommended that you always ship the release PDBs with the application.  
# PDBs can get pretty big (especially for full debug info) so, at least in the old days, installing them could add many MBs to the install size.  
# Therefore I generally follow the approach of shipping them on the installation CD in a directory that is not compressed.  If a customer reports 
# a problem that I can't easily replicate then I ask the customer to copy the PDB directory from the installation media to the program directory 
# and then try replicating the problem.  The generated Doc Watson log (or the crash dump even) will then give me more information to work with.
# 
# For applications that crash a lot (and that probably shouldn't have shipped) or in cases where you do Internet installs then installing the files 
# as part of a normal installation is reasonable.  You could even add them as a component in the installer such that by default they are not installed 
# but the user could later go back and rerun the install and add the debug component (containing the PDBs) if problems occur.
# 
# So, in summary, generating PDBs in any configuration is a good thing.  You should provide some mechanism for your clients to install the PDBs into 
# their program directory if something goes wrong.  You can either do this automatically, provide an option in the installer or simply provide 
# a version on the installation media.  IMHO.
#
# The /DEBUG:FASTLINK option leaves private symbol information in the individual compilation products used to build the executable. 
# It generates a limited PDB that indexes into the debug information in the object files and libraries used to build the executable 
# instead of making a full copy. This option can link from two to four times as fast as full PDB generation, and is recommended when 
# you are debugging locally and have the build products available. This limited PDB can't be used for debugging when the required 
# build products are not available, such as when the executable is deployed on another computer. In a developer command prompt, 
# you can use the mspdbcmf.exe tool to generate a full PDB from this limited PDB. In Visual Studio, use the Project or Build menu 
# items for generating a full PDB file to create a full PDB for the project or solution. 
#
# The /DEBUG:FULL option moves all private symbol information from individual compilation products (object files and libraries) 
# into a single PDB, and can be the most time-consuming part of the link. However, the full PDB can be used to debug the executable 
# when no other build products are available, such as when the executable is deployed. 
#
# When you specify /DEBUG with no additional options, the linker defaults to /DEBUG:FULL for command line and makefile builds, 
# for release builds in the Visual Studio IDE, and for both debug and release builds in Visual Studio 2015 and earlier versions. 
# Beginning in Visual Studio 2017, the build system in the IDE defaults to /DEBUG:FASTLINK when you specify the /DEBUG option for 
# debug builds. Other defaults are unchanged to maintain backward compatibility. 
#
# /ignore:4099 PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info
# Do ignore this linker warning if 3'rd party library doesn't provide debug information in the release mode.
#
# https://stackoverflow.com/questions/274034/given-a-crash-offset-pdb-and-source-how-can-i-find-the-source-line
# Open your binary using:
# WinDbg -z somebin.dll
# And in the command window, type:
# ln <address>
# Once you find the function it's in, run 
# uf somebin!SomeFunc
# to find the closest line to the address.
function(set_project_compile_flags)
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL /DEBUG:FASTLINK /SUBSYSTEM:WINDOWS /SAFESEH:NO /LARGEADDRESSAWARE")
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/INCREMENTAL:NO /DEBUG:FULL /SUBSYSTEM:WINDOWS /SAFESEH:NO /MAP /ignore:4099 /LARGEADDRESSAWARE")
endfunction(set_project_compile_flags)

function(set_console_project_compile_flags)
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL /DEBUG:FASTLINK /SUBSYSTEM:CONSOLE /SAFESEH:NO /LARGEADDRESSAWARE")
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/INCREMENTAL:NO /DEBUG:FULL /SUBSYSTEM:CONSOLE /SAFESEH:NO /MAP /ignore:4099 /LARGEADDRESSAWARE")
endfunction(set_console_project_compile_flags)

function(set_project_compile_options_debugrelease)
    IF( ${WIN32} ) 		
        list(APPEND COMPILEOPT_DEBUG
            /Od           	# Enable basic run-time checks
            /MDd		  	# Multithreaded debug runtime
        )

        list(APPEND COMPILEOPT_RELEASE
            /O2           	# Optimize for maximum speed
            /MD           	# Multithreaded release runtime
            /Ob1		  	# Inline function expansion
            /GF			  	# Enable string pooling
        )
    ELSE()
        list(APPEND COMPILEOPT_DEBUG -g -fPIC -shared -std=c++20)
        list(APPEND COMPILEOPT_RELEASE -fPIC -shared -export-dynamic -O2 -std=c++20)
    ENDIF()
	
	target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:${COMPILEOPT_DEBUG}>)
	target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:${COMPILEOPT_RELEASE}>)
endfunction(set_project_compile_options_debugrelease)

function(set_project_compile_options)
    IF( ${WIN32} ) 	
        list(APPEND COMPILEOPT
            /Gy			  	# Enable function level linking
            /Yu			  	# Use precompiled headers
            #/Zm800			# Precompiled header memory allocation limit. Disabled for #43854 - W40: /Zm800 # Precompiled header memory allocation limit => can be removed?
            /GR				# Enable run type information
            /std:c++20
            /JMC			# Enables Just My Code stepping, which brings better debuging experience reference #32706 for more information
        )
    ELSE()
        list(APPEND COMPILEOPT -std=c++20)
    ENDIF()
	
	target_compile_options(${PROJECT_NAME} PUBLIC  ${COMPILEOPT})	
	set_project_compile_options_debugrelease()
endfunction(set_project_compile_options)

#Should only be used if there is a requirement for order in the compile options.
function(set_additional_compile_option compile_opt)
	list(APPEND ADDITIONAL_COMPILEOPT
		"additional_compile_option${compile_opt}"
	)
	target_compile_options(${PROJECT_NAME} PUBLIC  ${ADDITIONAL_COMPILEOPT})	
endfunction(set_additional_compile_option)

function(set_project_processor_definitions)
    IF( ${WIN32} )	
        list (APPEND COMPILER_DEFINES
            _WINDOWS
        )
    ENDIF()

	list(APPEND COMPILER_DEFINES_DEBUG
		_DEBUG                    	
	)

	list(APPEND COMPILER_DEFINES_RELEASE
		NDEBUG                  
	)
	
	target_compile_definitions(${PROJECT_NAME} PUBLIC  ${COMPILER_DEFINES})
	target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:${COMPILER_DEFINES_DEBUG}>)
	target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:${COMPILER_DEFINES_RELEASE}>)
endfunction(set_project_processor_definitions)

function(set_project_definitions)
	#Unicode character set
	add_definitions(-DUNICODE -D_UNICODE)

	# Produces a program database (PDB) that contains type information and symbolic debugging information for use with the debugger. 
	# The symbolic debugging information includes the names and types of variables, as well as functions and line numbers. 
	# /Zi does not affect optimizations. However, /Zi does imply /debug; see /DEBUG (Generate Debug Info) for more information. 
	IF( ${WIN32} ) 
        add_definitions(/Zi)
    ENDIF()

    IF( ${WIN32} ) 
        #Multi process compilation (currently default and cpu2 supported)
        IF(EXISTS "${RootDir}/Tools/CMake/cpu2")
            add_definitions(/MP2)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu3")
            add_definitions(/MP3)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu4")
            add_definitions(/MP4)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu5")
            add_definitions(/MP5)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu6")
            add_definitions(/MP6)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu7")
            add_definitions(/MP7)  
        ELSEIF(EXISTS "${RootDir}/Tools/CMake/cpu8")
            add_definitions(/MP8)  
        ELSE()
            add_definitions(/MP)
        ENDIF()
    ENDIF()
    
	#64bit define
	IF( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8" )
		IF( ${WIN32} ) 
			add_definitions(-D_WIN64)
		ENDIF()
		add_definitions(-D__x86_64__)
	ENDIF()
		
endfunction(set_project_definitions)

function(set_project_bin_properties)
	set(RuntimeOutputNameDebug "${PROJECT_NAME}D")
	
	set_target_properties(${PROJECT_NAME} PROPERTIES 
						RUNTIME_OUTPUT_NAME_DEBUG ${RuntimeOutputNameDebug}
						RUNTIME_OUTPUT_DIRECTORY_DEBUG "${RootDir}/bin")
					  
	set_target_properties(${PROJECT_NAME} PROPERTIES 
						RUNTIME_OUTPUT_NAME_RELEASE ${PROJECT_NAME}
						RUNTIME_OUTPUT_DIRECTORY_RELEASE "${RootDir}/bin")
endfunction(set_project_bin_properties)

function(set_project_bin_lib_properties LibDirPath)
	set_project_bin_properties("${RootDir}/bin")
	set(LibraryOutputNameDebug "${PROJECT_NAME}D")
	set(LibraryOutputNameRelease "${PROJECT_NAME}")
	
	set_target_properties(${PROJECT_NAME} PROPERTIES 
						ARCHIVE_OUTPUT_NAME_DEBUG ${LibraryOutputNameDebug}
						ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LibDirPath})   #Import Library path
					  
	set_target_properties(${PROJECT_NAME} PROPERTIES 
						ARCHIVE_OUTPUT_NAME_RELEASE ${LibraryOutputNameRelease} 
						ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LibDirPath}) #Import Library path
endfunction(set_project_bin_lib_properties)
