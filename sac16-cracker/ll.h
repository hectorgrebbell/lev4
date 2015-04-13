/* Header for a quick linked list implementation */

struct key_l
{
	uint16_t key;
	uint64_t score;
	struct key_l *next, *prev;
};

struct key_l *add_key(struct key_l **h, struct key_l *t, uint16_t key, uint64_t score);

void remove_key(struct key_l **h, struct key_l **t, struct key_l *r);

struct key_l *getBest(uint16_t key, struct key_l **s, struct key_l *h);
