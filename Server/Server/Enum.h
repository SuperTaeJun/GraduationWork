#pragma once

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