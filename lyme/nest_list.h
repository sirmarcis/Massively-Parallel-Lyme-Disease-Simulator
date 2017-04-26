#ifndef __NESTLISTH_
#define __NESTLISTH_

#include <structs.h>



nest_list *nest_list_create();
void nest_list_free(nest_list *l);
int nest_list_contains_p(nest_list *l, nest *li);

nest * nest_list_add_element(nest_list *l, nest *li);
nest * pop_nest_left(nest_list *l);


#endif
