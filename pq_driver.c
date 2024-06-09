#include <stdlib.h>

#include "libpq-fe.h"
#include "pq_driver.h"

void
print_array (const char *strs[], int num_strs)
{
  printf ("%s", strs[0]);
  for (int i = 1; i < num_strs; i++)
    printf (" %s", strs[i]);
  printf("\n");
}

tuple *
pack_tuple (PGresult *res, int row)
{
  tuple *tup = (tuple *) malloc(sizeof (tuple));
  tup->num_fields = PQnfields(res);
  
  for (int i = 0; i < tup->num_fields; i++)
    {
      tup->attributes[i] = (char *) PQfname(res, i);
      tup->values[i] = (void *) PQgetvalue (res, row, i);
    }
  
  return tup;
}

void
print_tuple (tuple *tup)
{
  print_array (tup->attributes, tup->num_fields);
}

void
print_attributes (PGresult *res)
{
  for (int i = 0; i < PQnfields(res); i++)
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

  int num_cols = PQnfields(res);
  printf ("Number of columns: %d\n\n", num_cols);
  print_attributes(res);
    
  for (int i = 0; i < PQntuples(res); i++)
  {
    tuple *tup = pack_tuple(res, i);
    print_tuple(tup);
    free(tup);
  }
  printf ("\n");

  PQclear (res);
}

void
dump_query (PGconn *conn, const char *query)
{
  result_info (PQexec(conn, query));
}

PGconn *
get_conn(const char *params)
{
  return PQconnectdb(params);
}
