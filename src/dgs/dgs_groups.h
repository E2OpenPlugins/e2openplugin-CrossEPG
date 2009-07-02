#ifndef _DGS_GROUPS_H_
#define _DGS_GROUPS_H_

typedef struct dgs_group_s
{
	int 			 	id;
	char 			 	*name;
	int					seq;
	struct dgs_group_s	*prev;
	struct dgs_group_s	*next;
} dgs_group_t;

void dgs_groups_init ();
void dgs_groups_read ();
void dgs_groups_clean ();
dgs_group_t *dgs_groups_get_first ();
dgs_group_t *dgs_groups_get_last ();
dgs_group_t *dgs_groups_get_by_id (int grp_id);
dgs_group_t *dgs_groups_get_next (dgs_group_t *group);
dgs_group_t *dgs_groups_get_prev (dgs_group_t *group);
int dgs_groups_count ();

#endif // _DGS_GROUPS_H_
