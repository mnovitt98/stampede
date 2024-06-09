#include <stdlib.h>
#include <libguile.h>
#include "libpq-fe.h"

#include "pq_driver.h"

#define MAX_PARAMS 50

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
  int num_fields;    
  int num_results;
  ExecStatusType status_code;
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
  qr->num_results = PQntuples(qr->pg_res);
  qr->num_fields  = PQnfields(qr->pg_res);
  qr->status_code = PQresultStatus(qr->pg_res);  

  return scm_make_foreign_object_1 (postgres_result_type, qr);
}

static SCM
make_result_with_params (SCM conn_obj, SCM query, SCM params)
{
  /* coerce values */
  scm_assert_foreign_object_type (postgres_conn_type, conn_obj);
  db_connection *conn = scm_foreign_object_ref (conn_obj, 0);
  char *Query = scm_to_locale_string (query) ;

  /* allocate */
  query_result *qr = (query_result *)
    scm_gc_malloc (sizeof (query_result), "query_result");

  int nParams = scm_to_int (scm_length (params));

  static Oid  *paramTypes[MAX_PARAMS];
  static const char * const *paramValues[MAX_PARAMS];
  static int  paramLenghts[MAX_PARAMS];
  static int  paramFormats[MAX_PARAMS];

  /* load arrays */
  for (int i = 0; i < nParams; i++)
    {
      SCM param = scm_list_ref (params, scm_from_int (i));
      paramValues[i] = (const char * const *)
        scm_to_locale_string (param);
    }
  
  /* call and pack */
  qr->pg_res = PQexecParams(conn->pg_conn,
                            Query,
                            nParams,
                            NULL, /* paramTypes */
                            paramValues,
                            NULL, /* paramLenghts */
                            NULL, /* paramFormats */
                            0);
  qr->num_results = PQntuples(qr->pg_res);
  qr->num_fields  = PQnfields(qr->pg_res);
  qr->status_code = PQresultStatus(qr->pg_res);

  return scm_make_foreign_object_1 (postgres_result_type, qr);
}

static SCM
good_result (SCM result_obj)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  return scm_from_bool (   qr->status_code == PGRES_TUPLES_OK
                        || qr->status_code == PGRES_COMMAND_OK);
}

static SCM
get_result_ntuples (SCM result_obj)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  return scm_from_int (qr->num_results);
}

static SCM
get_result_nfields (SCM result_obj)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  return scm_from_int (qr->num_fields);
}

/* perform postgres data type to scm type here */
static SCM
get_cons_rep (const tuple *tup, const PGresult *res, int row, int col)
{
  SCM val = SCM_UNSPECIFIED;
  if (PQgetisnull(res, row, col))
    val = scm_list_n (SCM_UNDEFINED);
  else
    val = scm_from_locale_string ((char *) tup->values[col]);
                  
  return scm_cons (scm_from_locale_string ((char *) tup->attributes[col]), val);
}

static SCM
get_result_tuple (SCM result_obj, SCM row)
{
  scm_assert_foreign_object_type (postgres_result_type, result_obj);
  query_result *qr = scm_foreign_object_ref (result_obj, 0);

  int row_i = scm_to_int(row);

  tuple *tup = pack_tuple(qr->pg_res, row_i);
  SCM _list  = scm_list_n (SCM_UNDEFINED);
  for (int i = tup->num_fields - 1; i >= 0; i--)
    {
      _list = scm_cons (get_cons_rep (tup, qr->pg_res, row_i, i), _list);
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
  scm_c_define_gsubr ("get-result", 2, 0, 0, make_result);
  scm_c_define_gsubr ("get-presult", 3, 0, 0, make_result_with_params);  
  scm_c_define_gsubr ("get-nth", 2, 0, 0, get_result_tuple);
  scm_c_define_gsubr ("result-length", 1, 0, 0, get_result_ntuples);
  scm_c_define_gsubr ("result-fields", 1, 0, 0, get_result_nfields);
  scm_c_define_gsubr ("good-res?", 1, 0, 0, good_result);
}
