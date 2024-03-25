#pragma once
#ifndef __ENUM_H__
#define __ENUM_H__


enum EVENT_TYPE { CL_BONEFIRE };
enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
	//IO_CONNECT,
};
enum CL_STATE { ST_FREE, ST_ACCEPT, ST_INGAME };



#endif // !__ENUM_H__
