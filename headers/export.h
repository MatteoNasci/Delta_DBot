#pragma once
#ifndef H_MLN_DB_EXPORT_H
#define H_MLN_DB_EXPORT_H

#ifdef _WIN32
    #ifdef MLN_BOT_LIBRARY_EXPORTS
        #define MLN_BOT_LIBRARY_API __declspec(dllexport)
    #else //MLN_BOT_LIBRARY_EXPORTS
        #define MLN_BOT_LIBRARY_API __declspec(dllimport)
    #endif //MLN_BOT_LIBRARY_EXPORTS
#else //_WIN32
    #define MLN_BOT_LIBRARY_API
#endif //_WIN32

#endif //H_MLN_DB_EXPORT_H