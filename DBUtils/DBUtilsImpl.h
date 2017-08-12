#ifndef __DB_UTILS_IMPL_H__
#define __DB_UTILS_IMPL_H__
#pragma once
	
#ifdef _BUILD_DB_UTILS_IMPL
	#define DB_UTILS_API __declspec(dllexport)
#else
	#define DB_UTILS_API __declspec(dllimport)

	#ifdef _DEBUG
        #define DB_UTILS_LIB "DBUtilsImplUD.lib"
	#else
        #define DB_UTILS_LIB "DBUtilsImplU.lib"
	#endif

	#pragma message("DBUtilsImpl will automatically link with " DB_UTILS_LIB)
	#pragma comment(lib, DB_UTILS_LIB)  
#endif

#endif