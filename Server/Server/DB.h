#pragma once
#include <windows.h>
#include <sqlext.h>
#include <iostream>
#include <string>

void show_err();
void printSQLError(SQLHANDLE handle, SQLSMALLINT type);
void save_data(const char* id, const char* pw);
bool DB_odbc(const char* name, const char* pw);
bool DB_id(const char* login_id);
