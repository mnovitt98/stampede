#include "libpq-fe.h"
#include "pq_driver.h"

void
print_row_info (PGresult *res, int row, int numCols)
{
  printf ("ROW INFO\n\n");
  for (int i = 0; i < numCols; i++)
    {
      printf ("%s ", PQfname (res, i));
      printf ("%s\n", PQgetvalue (res, row, i));
    }
  printf ("\n");
}

void
result_info (PGresult *res)
{
  printf ("RESULT OVERVIEW\n\n");
  printf ("%s\n", PQresStatus(PQresultStatus(res)));
  printf ("Number of tuples: %d\n\n", PQntuples(res));

  int numCols = PQnfields(res);
  printf ("Number of columns: %d\n", numCols);
  
  for (int i = 0; i < PQntuples(res); i++)
    print_row_info(res, i, numCols);
  printf ("\n");

  PQclear (res);
}

void
dump_query (PGconn *conn, const char *query)
{
  result_info (PQexec(conn, query));
}

PGconn
*get_conn(const char *params)
{
  return PQconnectdb(params);
}

