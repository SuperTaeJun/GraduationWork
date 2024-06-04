#include "pch.h"
#include "DB.h"
#include <sqlext.h>
#include <iostream>
#include <string>
void show_err() {
	cout << "error" << endl;
}

void printSQLError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLWCHAR sqlState[1024];
    SQLWCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT msgLength;
    SQLRETURN ret;
    SQLSMALLINT i = 1;
    while ((ret = SQLGetDiagRec(type, handle, i, sqlState, &nativeError, message, 1024, &msgLength)) != SQL_NO_DATA) {
        std::wcerr << L"SQL error: " << message << L", SQL state: " << sqlState << L", Native error: " << nativeError << std::endl;
        i++;
    }
}

void save_data() {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0;
    SQLRETURN retcode;

    wchar_t temp[BUFSIZE] = {};
    swprintf_s(temp, L"EXEC [GraduationWork].[dbo].create_user_data N'%s', N'%s'", L"joonhyn", L"05313");

    // 환경 핸들 할당
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // ODBC 버전 설정
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        // 연결 핸들 할당
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // 로그인 타임아웃 설정 (5초)
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // 데이터 소스에 연결
                retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2024GraduationWork", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

                // 문장 핸들 할당
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    std::wcout << L"ODBC Connection Success" << std::endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)temp, SQL_NTS);
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                        std::wcout << L"Data inserted successfully" << std::endl;
                    }
                    else {
                        std::wcerr << L"Error executing SQL statement" << std::endl;
                        printSQLError(hstmt, SQL_HANDLE_STMT);
                    }

                    // 문장 핸들 해제
                    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                }
                else {
                    std::wcerr << L"ODBC Connection Failed" << std::endl;
                    printSQLError(hdbc, SQL_HANDLE_DBC);
                }

                // 연결 해제
                SQLDisconnect(hdbc);
            }
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        }
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}
