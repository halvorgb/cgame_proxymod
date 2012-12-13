#include <stdlib.h>
#include "cg_local.h"
#include "cg_cvar.h"
#include "cg_utils.h"



vmCvar_t mdd_cgameproxy_version;
vmCvar_t mdd_hud_draw;
vmCvar_t mdd_hud_opacity;

vmCvar_t mdd_hud_ammo_draw;
vmCvar_t mdd_hud_ammo_offsetX;
vmCvar_t mdd_hud_ammo_offsetY;
vmCvar_t mdd_hud_ammo_size;

static cvarTable_t cvarTable[] = {
	{ &mdd_cgameproxy_version, "mdd_cgameproxy_version", VERSION, CVAR_USERINFO|CVAR_INIT },
	{ &mdd_hud_draw,           "mdd_hud_draw",           "1",     CVAR_ARCHIVE },
	{ &mdd_hud_opacity,        "mdd_hud_opacity",        "0.5",   CVAR_ARCHIVE },

	{ &mdd_hud_ammo_draw,      "mdd_hud_ammo_draw",      "0",     CVAR_ARCHIVE },
	{ &mdd_hud_ammo_offsetX,   "mdd_hud_ammo_offsetX",   "620",   CVAR_ARCHIVE },
	{ &mdd_hud_ammo_offsetY,   "mdd_hud_ammo_offsetY",   "30",    CVAR_ARCHIVE },
	{ &mdd_hud_ammo_size,      "mdd_hud_ammo_size",      "32",    CVAR_ARCHIVE }
};



int8_t cvar_register( char *name, char *value ) {
	return 0;
}



int8_t cvar_getFloat( const char *var_name, float *value ) {
	char buffer[1024];

	g_syscall( CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, 256 );
	*value = atof( buffer ); // can't tell if 0 or ERROR
	// TODO: make this return qfalse in case of error -> no atof()

	return qtrue;
}



int8_t init_cvars( void ) {
	int i=0;
	cvarTable_t *cv;
	uint32_t cvarTableSize;

	cvarTableSize = ARRAY_LEN( cvarTable );

	cv = cvarTable;
	for ( i=0; i < cvarTableSize; i++, cv++ ) {
		g_syscall( CG_CVAR_REGISTER, cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}
	return qtrue;
}
