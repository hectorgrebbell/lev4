/* A quick hacky linked list implementation */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "ll.h"

struct key_l *add_key(struct key_l **h, struct key_l *t, uint16_t key, uint64_t score)
{
	if ((*h) == NULL)
	{
		*h = (struct key_l*)malloc(sizeof(struct key_l));
		(*h)->key = key;
		(*h)->score = score;
		(*h)->next = NULL;
		(*h)->prev = NULL;
		t = *h;
	}
	else
	{
		t->next = (struct key_l*)malloc(sizeof(struct key_l));
		t->next->prev = t;
		t = t->next;
		t->key = key;
		t->next = NULL;
		t->score = score;
	}
	return t;
}

void remove_key(struct key_l **h, struct key_l **t, struct key_l *r)
{
	if (*h == NULL || *t == NULL || r == NULL)
	{
		printf("Invalid Argument to Remove_Key");
	}
	else if (r->prev == NULL)
	{
		if (r->next == NULL)
		{
			free(r);
			*h = NULL;
			*t = NULL;
		}
		else
		{
			*h = r->next;
			r->next->prev = NULL;
			free(r);
		}
	}
	else if (r->next == NULL)
	{
		*t = r->prev;
		r->prev->next = NULL;
		free(r);
	}
	else
	{
		r->prev->next = r->next;
		r->next->prev = r->prev;
		free(r);
	}
}

struct key_l *getBest(uint16_t key, struct key_l **s, struct key_l *h)
{
	uint64_t max = 0;
	struct key_l *c = h, *r = NULL;
	while (c != NULL)
	{
		if (c->score > max && c->key != key)
		{
			max = c->score;
			*s = r;
			r = c;
		}
		c = c->next;
	}
	return r;
}

