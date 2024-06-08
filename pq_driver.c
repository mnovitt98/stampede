#include <stdlib.h>

#include "libpq-fe.h"
#include "pq_driver.h"

tuple *
pack_tuple (PGresult *res, int row)
{
  tuple *tup = (tuple *) malloc(sizeof (tuple));

  tup->numFields = PQnfields(res);
  for (int i = 0; i < tup->numFields; i++)
    {
      tup->fields[i] = (void *) PQgetvalue (res, row, i);
    }
  
  return tup;
}

void
print_tuple (tuple *tup)
{
  for (int i = 0; i < tup->numFields; i++)
    printf ("%s ", (char *) tup->fields[i] );
  printf("\n");
}

void
print_row_info (PGresult *res, int row, int numCols)
{
  printf ("ROW INFO\n");
  for (int i = 0; i < numCols; i++)
    {
      printf ("%s ", PQfname (res, i));
    }
  printf ("\n");
}

void
result_info (PGresult *res)
{
  printf ("RESULT OVERVIEW\n\n");
  printf ("%s\n", PQresStatus(PQresultStatus(res)));
  printf ("Number of tuples: %d\n", PQntuples(res));

  int numCols = PQnfields(res);
  printf ("Number of columns: %d\n\n", numCols);
  print_row_info(res, 0, numCols);
    
  for (int i = 0; i < PQntuples(res); i++)
  {
    print_tuple(pack_tuple(res, i)); /* this is a memory leak lol */
  }
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

