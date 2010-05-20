#include "quake2/g_local.h"

void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if(!ent->inuse || ent->client->botinfo.active)
	  return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.centerprintf(ent, bigbuffer);
}

void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if (ent  &&  (!ent->inuse  ||  ent->client->botinfo.active))
		return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.cprintf(ent, printlevel, bigbuffer);
}

void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char bigbuffer[0x1000];
	int  len;
	va_list  argptr;
	edict_t *cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);


	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if(cl_ent->inuse && cl_ent->client && !cl_ent->client->botinfo.active)
			gi.cprintf(cl_ent, printlevel, bigbuffer);
	}
}

