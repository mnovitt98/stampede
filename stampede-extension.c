#include <stdlib.h>
#include <libguile.h>
#include "libpq-fe.h"

#include "pq_driver.h"

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

void
init_stampede(void)
{
  init_postgres_conn_type ();

  scm_c_define_gsubr ("make-connection", 1, 0, 0, make_connection);
  scm_c_define_gsubr ("good-conn?", 1, 0, 0, check_connection_status);
  scm_c_define_gsubr ("dump-exec", 2, 0, 0, dump_stmt_res);
}




