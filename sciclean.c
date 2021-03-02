#include <sciclean.h>
#include <string.h>
#include <stdlib.h>

struct sciclean {
	void *obj;
	void (*obj_clean)(void*);
};

struct sciclean_list {
	int             tot;
	int             len;
	struct sciclean *l;
};

#define INCR 10

/* Register a cleanup; a NULL obj_clean function
 * may be used which is equivalent to 'free'.
 */
Scicleanup
sciclean_push(Sciclean context, void *obj, void (*obj_clean)(void *obj))
{
struct sciclean_list *c = context;

	/* default */
	if ( !obj_clean )
		obj_clean = free;

	if ( !obj )
		return -1;
	
	if ( ++(c->len) >= c->tot ) {
		/* need more space -- assume we get it */
		c->tot += INCR;
		c->l = realloc(c->l, sizeof(*c->l) * (c->tot));
	}
	c->l[c->len].obj       = obj;
	c->l[c->len].obj_clean = obj_clean;
	return c->len;
}

/* Cancel a cleanup */
void
sciclean_cancel(Sciclean context, Scicleanup cleanup)
{
struct sciclean_list *c = context;

	if ( cleanup < 0 || cleanup > c->len )
		return; /* silently ignore invalid argument */

	c->l[cleanup].obj       = 0;
	c->l[cleanup].obj_clean = 0;

	/* Try to recycle if this was the last slot */
	if ( c->len == cleanup )
		c->len--;
}

/* Scilab interface function */
int
sciclean_trampoline(char *fname, void *uarg, ScicleanGatefunc f)
{
int    i;
struct sciclean_list context;
struct sciclean *p;
	
	/* initialize context */
	context.l   = 0;
	context.tot = 0;
	context.len = -1;

	/* call user interface function */
	(*f)(fname, uarg, &context);

	/* cleanup */
	for ( i=0, p=context.l; i<=context.len; i++,p++ ) {
		if ( p->obj_clean )
			p->obj_clean(p->obj);
	}
	/* clean list itself */
	free(context.l);

	return 0;
}
