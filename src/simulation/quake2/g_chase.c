#include "g_local.h"

void UpdateChaseCam(edict_t *ent)
{
 // -- Fear Begin --

	vec3_t o, ownerv, 
		goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	vec3_t oldgoal;
	vec3_t angles;

	// is our chase target gone?
	if (!ent->client->chase_target->inuse
		|| ent->client->chase_target->client->resp.spectator) {
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old) {
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	targ = ent->client->chase_target;
/*
	trace = gi.trace(ent->s.origin, vec3_origin, vec3_origin, targ->s.origin, targ, MASK_SOLID);

	if (trace.fraction < 1.0f)
	{
		float dist = 0.0f;
		int count = 0;
		vec3_t mins = { -16, -16, -16 }, maxs = { 16, 16, 16 };
		while (dist<108.0f  &&  count < 8)
		{
			AngleVectors( targ->client->ps.viewangles, forward, NULL, NULL );
			forward[2] += 0.2f + random() * 1.0f;
			forward[1] += crandom();
			forward[0] += crandom();
			VectorNormalize2( forward, forward );
			VectorMA( ent->s.origin, 540, forward, goal );
			trace = gi.trace(targ->s.origin, mins, maxs, goal, targ, MASK_SOLID);
			dist = trace.fraction * 540 - 54;
			count++;
		}
		VectorMA( targ->s.origin, dist, forward, ent->s.origin );
		VectorSubtract( targ->s.origin, ent->s.origin, goal );
		vectoangles( goal, forward );
		VectorCopy( forward, ent->client->ps.viewangles );
	}

	VectorSubtract( targ->s.origin, ent->s.origin, goal );
	vectoangles( goal, forward );

	for (i=0 ; i<3 ; i++)
	{
		while (forward[i]-ent->client->ps.viewangles[i]<-180.0f) forward[i] += 360.0f;
		while (forward[i]-ent->client->ps.viewangles[i]>180.0f) forward[i] -= 360.0f;
		forward[i] = ent->client->ps.viewangles[i] + (forward[i] - ent->client->ps.viewangles[i]) * 0.25f;
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i] - ent->client->resp.cmd_angles[i]);
		ent->client->ps.viewangles[i] = forward[i];
		ent->client->v_angle[i] = forward[i];
	}

	ent->client->ps.pmove.pm_type = PM_FREEZE;
*/

	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(ent->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -30, forward, o);

	if (o[2] < targ->s.origin[2] + 20)
		o[2] = targ->s.origin[2] + 20;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	if (targ->deadflag)
		ent->client->ps.pmove.pm_type = PM_DEAD;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (targ->deadflag) {
		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
	} else {
		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
	}

 // -- Fear End --

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);
}

void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!e->client->resp.spectator)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void GetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && !other->client->resp.spectator) {
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam(ent);
			return;
		}
	}
	safe_centerprintf(ent, "No other players to chase.");
}

