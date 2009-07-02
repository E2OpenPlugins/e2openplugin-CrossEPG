#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "db/db.h"
#include "directory.h"

#include "../common.h"
#include "../common/core/log.h"

#include "dgs.h"
#include "dgs_groups.h"

static dgs_group_t *first_group;
static dgs_group_t *last_group;
static int groups_count;

void dgs_groups_init ()
{
	first_group = NULL;
	last_group = NULL;
	groups_count = 0;
}

void dgs_groups_clean ()
{
	dgs_group_t *tmp = first_group;
	
	while (tmp != NULL)
	{
		dgs_group_t *tmp2 = tmp;
		_free (tmp2->name);
		_free (tmp2);
		tmp = tmp->next;
	}
}

int _dgs_groups_read_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int *count = (int*)p_data;
	
	dgs_group_t *new_group = _malloc (sizeof (dgs_group_t));
	
	if (first_group == NULL)
	{
		new_group->prev = NULL;
		new_group->next = NULL;
		first_group = new_group;
		last_group = new_group;
	}
	else
	{
		new_group->prev = last_group;
		new_group->next = NULL;
		last_group->next = new_group;
		last_group = new_group;
	}
	
	last_group->name = _malloc (strlen (p_fields[1]) + 1);
	strcpy (last_group->name, p_fields[1]);
	last_group->id = atoi (p_fields[0]);
	last_group->seq = atoi (p_fields[2]);
	
	*count = *count + 1;
    return 0;
}

void dgs_groups_read (int id)
{
    int ret;
	int count = 0;
    char *errmsg = 0;

	if (dgs_getdb () == NULL) return;

	ret = sqlite3_exec (dgs_getdb (), "SELECT id, name, seq FROM FAVGRPInfo ORDER BY seq", _dgs_groups_read_callback, &count, &errmsg);
	
	if (ret == SQLITE_OK)
		groups_count = count;
	else
		log_add ("SQL error: %s", errmsg);
}

dgs_group_t *dgs_groups_get_first ()
{
	return first_group;
}

dgs_group_t *dgs_groups_get_last ()
{
	return last_group;
}

dgs_group_t *dgs_groups_get_by_id (int grp_id)
{
	dgs_group_t *tmp = first_group;
	
	while (tmp != NULL)
	{
		if (tmp->id == grp_id)
			return tmp;
			
		tmp = tmp->next;
	}
	
	return first_group;
}

dgs_group_t *dgs_groups_get_next (dgs_group_t *group)
{
	if (group != NULL)
	{
		if (group->next != NULL) return group->next;
		else return first_group;
	}
	return NULL;
}

dgs_group_t *dgs_groups_get_prev (dgs_group_t *group)
{
	if (group != NULL)
	{
		if (group->prev != NULL) return group->prev;
		else return last_group;
	}
	return NULL;
}

int dgs_groups_count ()
{
	return groups_count;
}
