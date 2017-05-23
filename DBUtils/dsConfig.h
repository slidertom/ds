#ifndef __DS_CONFIG_H__
#define __DS_CONFIG_H__
#pragma once

#include "string"

namespace ds_log_file_path
{
    // LOG file default path
    inline std::wstring GetDefaultPath() {
       return L""; 
    }
};

#endif
