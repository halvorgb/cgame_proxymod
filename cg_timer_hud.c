#include <stdio.h>
#include "cg_local.h"
#include "cg_draw.h"
#include "cg_cvar.h"
#include "cg_utils.h"

#define	PMF_TIME_KNOCKBACK	64
#define	PMF_RESPAWNED		512

#define ET_MISSILE 3
#define MAX_GB_TIME 250

#define MAX_NADES 10
#define NADE_EXPLODE_TIME 2500

typedef struct {
	int id;
	int explode_time;
	int seen;
} nade_info_t;

static nade_info_t nades[MAX_NADES];

static vmCvar_t timer_draw;
static vmCvar_t timer_x;
static vmCvar_t timer_y;
static vmCvar_t timer_w;
static vmCvar_t timer_h;
static vmCvar_t timer_item_w;
static vmCvar_t timer_item_rgba;
static vmCvar_t timer_gb_rgba;
static vmCvar_t timer_outline_rgba;

static int find_nade(int nade_id);
static int track_nade(int nade_id, int time);
static void draw_outline(vec4_t color);
static void draw_item(float progress, vec4_t color);

static void init_timer_cvars(void) {
	// what are arrays?
	g_syscall(CG_CVAR_REGISTER, &timer_draw, "mdd_hud_timer_draw", "0", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_x, "mdd_hud_timer_x", "275", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_y, "mdd_hud_timer_y", "275", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_w, "mdd_hud_timer_w", "100", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_h, "mdd_hud_timer_h", "16", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_item_w, "mdd_hud_timer_item_w", "5", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_item_rgba, "mdd_hud_timer_item_rgba", "1 1 0 1", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_gb_rgba, "mdd_hud_timer_gb_rgba", "1 0 0 1", CVAR_ARCHIVE);
	g_syscall(CG_CVAR_REGISTER, &timer_outline_rgba, "mdd_hud_timer_outline_rgba", "1 1 1 1", CVAR_ARCHIVE);
}

void timer_hud_init(void) {
	init_timer_cvars();

	for (int i = 0; i < MAX_NADES; i++)
		nades[i].id = -1;
}

static void update_timer_cvars(void) {
	// what are arrays?
	g_syscall(CG_CVAR_UPDATE, &timer_draw);
	g_syscall(CG_CVAR_UPDATE, &timer_x);
	g_syscall(CG_CVAR_UPDATE, &timer_y);
	g_syscall(CG_CVAR_UPDATE, &timer_w);
	g_syscall(CG_CVAR_UPDATE, &timer_h);
	g_syscall(CG_CVAR_UPDATE, &timer_item_w);
	g_syscall(CG_CVAR_UPDATE, &timer_item_rgba);
	g_syscall(CG_CVAR_UPDATE, &timer_gb_rgba);
	g_syscall(CG_CVAR_UPDATE, &timer_outline_rgba);
}

void timer_hud_draw(void) {
	vec4_t outline_color;
	vec4_t item_color;
	vec4_t gb_color;
	snapshot_t *snap;
	playerState_t *ps;

	update_timer_cvars();

	if (!timer_draw.integer)
		return;

	sscanf(timer_outline_rgba.string, "%f %f %f %f",
			&outline_color[0], &outline_color[1],
			&outline_color[2], &outline_color[3]);

	sscanf(timer_item_rgba.string, "%f %f %f %f",
			&item_color[0], &item_color[1],
			&item_color[2], &item_color[3]);

	sscanf(timer_gb_rgba.string, "%f %f %f %f",
			&gb_color[0], &gb_color[1],
			&gb_color[2], &gb_color[3]);

	// draw the outline
	draw_outline(outline_color);

	snap = getSnap();
	ps = getPs();

	// gb stuff
	// todo: use pps if available
	if (ps->pm_flags & PMF_TIME_KNOCKBACK
		&& ps->groundEntityNum != ENTITYNUM_NONE
		&& !(ps->pm_flags & PMF_RESPAWNED))
	{
		float gb_progress = 1.0 - (float)ps->pm_time / MAX_GB_TIME;
		draw_item(gb_progress, gb_color);
	}

	// cull exploded nades, and set valid nades to not seen yet
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1)
			continue;

		if (nades[i].explode_time - snap->serverTime <= 0)
			nades[i].id = -1;
		else
			nades[i].seen = 0;
	}

	// traverse ent list to update nade tracking
	for (int i = 0; i < snap->numEntities; i++) {
		entityState_t entity = snap->entities[i];
		if (entity.eType == ET_MISSILE
			&& entity.weapon == WP_GRENADE_LAUNCHER
			&& entity.clientNum == ps->clientNum)
		{
			int nade_index = find_nade(entity.number);
			if (nade_index == -1)
				track_nade(entity.number, snap->serverTime);
			else
				nades[nade_index].seen = 1;
		}
	}

	// cull unseen nades and draw the rest
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1)
			continue;

		if (!nades[i].seen) {
			nades[i].id = -1;
		}
		else {
			float progress = 1.0 -
				(float)(nades[i].explode_time - snap->serverTime) / NADE_EXPLODE_TIME;
			draw_item(progress, item_color);
		}
	}
}

static int find_nade(int nade_id) {
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == nade_id)
			return i;
	}

	return -1;
}

static int track_nade(int nade_id, int time) {
	for (int i = 0; i < MAX_NADES; i++) {
		if (nades[i].id == -1) {
			nades[i].id = nade_id;
			nades[i].explode_time = time + NADE_EXPLODE_TIME;
			nades[i].seen = 1;
			return 0;
		}
	}

	// no free space to track the nade
	return -1;
}

static void draw_outline(vec4_t color) {
	int x = timer_x.integer;
	int y = timer_y.integer;
	int w = timer_w.integer;
	int h = timer_h.integer;

	g_syscall( CG_R_SETCOLOR, color );
	CG_DrawAdjPic(x, y - 1, w, 1, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x, y + h, w, 1, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x + w, y - 1, 1, h + 2, cgs.media.gfxWhiteShader);
	CG_DrawAdjPic(x - 1, y - 1, 1, h + 2, cgs.media.gfxWhiteShader);
}

static void draw_item(float progress, vec4_t color) {
	int x = timer_x.integer;
	int y = timer_y.integer;
	int w = timer_w.integer;
	int h = timer_h.integer;
	int i_w = timer_item_w.integer;

	g_syscall( CG_R_SETCOLOR, color );
	CG_DrawAdjPic(x + (w - i_w) * progress, y, i_w, h, cgs.media.gfxWhiteShader);
}