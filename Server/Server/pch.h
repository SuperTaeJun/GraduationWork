#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <thread>
#include <string>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <array>
#include <chrono>
#include <mutex>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <concurrent_priority_queue.h>
#include <windows.h>
#include <sqlext.h>
using namespace std;

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

#include "Enum.h"
#include "protocol.h"
#include "Overlap.h"
#include "DB.h"
#include "CLIENT.h"



void error_display(int err_no);