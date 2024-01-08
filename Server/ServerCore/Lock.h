#pragma once
#include "Types.h"

/*----------------
    RW SpinLock
-----------------*/

/*--------------------------------------------
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
---------------------------------------------*/

//write lock을 잡은 상태에선 read lock을 잡을수 있음
// write lock을 잡은 상태에서 다시 write을 잡을 수도 있음
//단 read lock을 잡은 상태에선 write lock을 잡으면 안됨

class Lock
{
    enum : uint32                        //32비트 정수에 16비트씩 나눠 관리 
    {
        ACQUIRE_TIMEOUT_TICK = 10000,    //최대 tick
        MAX_SPIN_COUNT = 5000,           //최대 spinlock 카운트
        WRITE_THREAD_MASK = 0xFFFF'0000, //write 하는 쓰레드 id    
        READ_COUNT_MASK = 0x0000'FFFF,   //read 카운트
        EMPTY_FLAG = 0x0000'0000
    };

public:
    void WriteLock();
    void WriteUnlock();
    void ReadLock();
    void ReadUnlock();

private:
    Atomic<uint32> _lockFlag = EMPTY_FLAG;
    uint16 _writeCount = 0;
};

/*----------------
    LockGuards
-----------------*/

class ReadLockGuard
{
public:
    ReadLockGuard(Lock& lock) : _lock(lock) { _lock.ReadLock(); }
    ~ReadLockGuard() { _lock.ReadUnlock(); }

private:
    Lock& _lock;
};

class WriteLockGuard
{
public:
    WriteLockGuard(Lock& lock) : _lock(lock) { _lock.WriteLock(); }
    ~WriteLockGuard() { _lock.WriteUnlock(); }

private:
    Lock& _lock;
};