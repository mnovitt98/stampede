#ifndef PG_INTERFACE_H
#define PG_INTERFACE_H

#include "libpq-fe.h"

void
print_row_info (PGresult *res, int row, int numCols);

void
result_info (PGresult *res);

void
dump_query (PGconn *conn, const char *query);

PGconn
*get_conn(const char *params);

#endif /* end of PG_INTERFACE_H */
