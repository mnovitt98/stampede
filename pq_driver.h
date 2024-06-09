#ifndef PG_INTERFACE_H
#define PG_INTERFACE_H

#include "libpq-fe.h"

#define MAX_TABLE_FIELDS 100

void
print_array (const char *strs[], int num_strs);

typedef struct
{
  int num_fields;
  void *attributes[MAX_TABLE_FIELDS];
  void *values[MAX_TABLE_FIELDS];  
} tuple;

tuple *
pack_tuple (PGresult *res, int row);

void
print_tuple (tuple *tup);

void
print_attributes (PGresult *res);

void
result_info (PGresult *res);

void
dump_query (PGconn *conn, const char *query);

PGconn
*get_conn(const char *params);

#endif /* end of PG_INTERFACE_H */
