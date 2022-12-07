#ifndef __LINE_H__
#define __LINE_H__

#include "stdint.h"
typedef struct line
{
    struct line *prev; //指向直接前趋
    struct line *next; //指向直接后继
    void *data;
} line;

line *line_create(void);
uint8_t line_add(line *head, void *data);
uint8_t line_inser(line *head, void *data, int add);
uint8_t line_inser_after(line *data, void *add);
uint8_t line_del_element(line *head, void *data);
line *line_search_element(line *head, void *data);
uint8_t line_del(line *head, uint8_t flag);
uint8_t line_is_empty(line *head);
void line_display(line *head);

#endif
