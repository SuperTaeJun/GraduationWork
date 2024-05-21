#pragma once
#ifndef __ENUM_H__
#define __ENUM_H__


enum EVENT_TYPE { ET_RELOAD, ET_HEAL };
enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
	IO_RELOAD_WEAPON,
	IO_HEAL_HP
};
enum CL_STATE { ST_FREE, ST_ACCEPT, ST_INGAME, ST_LOBBY };

#endif // !__ENUM_H__
