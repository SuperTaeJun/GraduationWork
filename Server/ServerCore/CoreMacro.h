#pragma once

#define OUT


/*---------------
	  Lock
---------------*/

//락을 여러개 사용할때
#define USE_MANY_LOCKS(count)	Lock _locks[count];
//락을 하나만 사용할때
#define USE_LOCK				USE_MANY_LOCKS(1)
// 몇번쨰에 read lock을 잡을 것이냐
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx]);
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx]);
#define WRITE_LOCK				WRITE_LOCK_IDX(0)



//define을 여러줄하기 위해서 \ 을 써야함

//=====================================================
//                Crash
//컴파일러한테 nullptr이 아니라고 가정하라 요청함
//뒤에 값을 넣어 일부러 crash 냄
//=====================================================

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

//===================================================
//             조건부 Crash 
//==================================================

#define ASSERT_CRASH(expr)			\
{									\
	if (!(expr))					\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}