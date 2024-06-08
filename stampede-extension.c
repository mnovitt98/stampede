


#include <stdlib.h>
#include <libguile.h>
#include "libpq-fe.h"

#include "pq_driver.h"

/* CONNECTION FOREIGN OBJECT */
typedef struct
{
  PGconn *pg_conn;
  SCM name;
} db_connection;

static SCM postgres_conn_type;

static void
init_postgres_conn_type (void)
{
  SCM name, slots;
  scm_t_struct_finalize finalizer;

  name = scm_from_utf8_symbol ("pg-conn");
  slots = scm_list_1 (scm_from_utf8_symbol ("data"));
  finalizer = NULL;

  postgres_conn_type =
    scm_make_foreign_object_type (name, slots, finalizer);
}

static SCM
make_connection (SCM init_string)
{
  char *params = scm_to_locale_string (init_string) ;

  db_connection *conn = (db_connection *)
    scm_gc_malloc (sizeof (db_connection), "db_connection");

  conn->pg_conn = get_conn (params); 

  return scm_make_foreign_object_1 (postgres_conn_type, conn);
}

static SCM
check_connection_status (SCM conn_obj)
{
  scm_assert_foreign_object_type (postgres_conn_type, conn_obj);
  db_connection *conn = scm_foreign_object_ref (conn_obj, 0);

  return scm_from_bool (PQstatus(conn->pg_conn) == CONNECTION_OK);
}

static SCM
dump_stmt_res (SCM conn_obj, SCM stmt)
{
  scm_assert_foreign_object_type ( postgres_conn_type,conn_obj);

  char *Stmt = scm_to_locale_string (stmt);
  db_connection *conn = scm_foreign_object_ref (conn_obj, 0);

  dump_query (conn->pg_conn, Stmt);

  return SCM_UNSPECIFIED;
}

/* END OF CONNECTION FOREIGN OBJECT TYPE */

/* RESULT FOREIGN OBJECT TYPE */

typedef struct
{
  PGresult *pg_res;
  int numFields;    
  int numResults;
  SCM name;
} query_result;

static SCM postgres_result_type;

static void
init_postgres_result_type (void)
{
  SCM name, slots;
  scm_t_struct_finalize finalizer;

  name = scm_from_utf8_symbol ("pg-res");
  slots = scm_list_1 (scm_from_utf8_symbol ("data"));
  finalizer = NULL;

  postgres_result_type =
    scm_make_foreign_object_type (name, slots, finalizer);
}

static SCM
make_result (SCM conn_obj, SCM query)
{
  scm_assert_foreign_object_type (postgres_conn_type, conn_obj);
  db_connection *conn = scm_foreign_object_ref (conn_obj, 0);
  char *Query = scm_to_locale_string (query) ;
  
  query_result *qr = (query_result *)
    scm_gc_malloc (sizeof (query_result), "query_result");

  qr->pg_res = PQexec(conn->pg_conn, Query);
  qr->numResults = PQntuples(qr->pg_res);
  qr->numFields  = PQnfields(qr->pg_res);  

  return scm_make_foreign_object_1 (postgres_result_type, qr);
}

static SCM
get_result_ntuples (SCM result_obj)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  return scm_from_int (qr->numResults);
}

static SCM
get_result_tuple (SCM result_obj, SCM row)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  int row_i = scm_to_int(row);

  tuple *tup = pack_tuple(qr->pg_res, row_i);
  SCM _list  = scm_list_n (SCM_UNDEFINED);
  for (int i = tup->numFields - 1; i >= 0; i--)
    {
      _list = scm_cons (scm_from_locale_string ((char *) tup->fields[i]), _list);
    }
  
  return _list;
}

/* END OF RESULT FOREIGN OBJECT TYPE */

void
init_stampede(void)
{
  init_postgres_conn_type ();
  init_postgres_result_type ();  

  scm_c_define_gsubr ("make-connection", 1, 0, 0, make_connection);
  scm_c_define_gsubr ("good-conn?", 1, 0, 0, check_connection_status);
  scm_c_define_gsubr ("dump-exec", 2, 0, 0, dump_stmt_res);
  scm_c_define_gsubr ("make-result", 2, 0, 0, make_result);
  scm_c_define_gsubr ("get-nth", 2, 0, 0, get_result_tuple);
  scm_c_define_gsubr ("result-length", 1, 0, 0, get_result_ntuples);
}
