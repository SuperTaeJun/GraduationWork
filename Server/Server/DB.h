#pragma once
#include "pch.h"


extern void show_err();
extern void printSQLError(SQLHANDLE handle, SQLSMALLINT type);
extern void save_data(const char* id, const char* pw);
extern bool DB_odbc(const char* name, const char* pw);
extern bool DB_id(const char* login_id);
