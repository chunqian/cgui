#ifndef ID_H
#define ID_H

#define DEFAULT_CHUNK_SIZE 10000

struct t_object;
extern void ReturnId(int id);
extern struct t_object *GetObject(int id);
extern int GetAnId(struct t_object *b);
extern void AdjustIdParam(int block_size, int maxlevel);

#endif
