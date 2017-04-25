#ifndef __LIST_H
#define __LIST_H

#include <structs.h>



mouse_list *mouse_list_create();
void mouse_list_free(mouse_list *l);

mouse * mouse_list_add_element(mouse_list *l, mouse *li);
mouse * pop_mouse_left(mouse_list *l);


#endif