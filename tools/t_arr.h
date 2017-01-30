#ifndef ARR_H
#define ARR_H

struct t_array;
struct t_header;
struct t_ifile;

extern void destroy_array(struct t_array *arr);
extern int build_array_header(struct t_header *hdr, struct t_array *arr, int i);
extern void build_array_intermediate(struct t_ifile *imed, struct t_array *arr);
extern void build_header_array_intermediate(struct t_ifile *imed, struct t_array *arr);

#endif
