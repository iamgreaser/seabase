// i was about to call this lua.c by accident --GM

// there is a very good reason why the above comment exists
// although there is no good reason why it STILL exists --GM

#include "common.h"

/**
	\brief Lua helper: Gets a userdata safely, blocking if data still loading.

	\param L Lua state.
	\param typ Expected userdata type.
	\param tname Name of userdata type.
	\param idx Index of variable to get userdata from.

	\return Userdata if successful and matching; throws Lua error otherwise.
*/
ud_t *ud_get_block(lua_State *L, int typ, char *tname, int idx)
{
	ud_t *ud = lua_touserdata(L, idx);

	if(ud == NULL)
		luaL_error(L, "type mismatch for %s", tname);

	if(ud->ud == UD_LOADING)
	{
		loading_t *ld = (loading_t *)(ud->v);

		if(ld->ud != typ)
			luaL_error(L, "type mismatch for %s", tname);

		// TODO: block
		fprintf(stderr, "TODO: block on UD_LOADING in ud_get_block\n");
		fflush(stderr);
		abort();
	}

	if(ud->ud != typ)
		luaL_error(L, "type mismatch for %s", tname);
	
	return ud;
}

