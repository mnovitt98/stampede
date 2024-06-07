guile_site = /opt/homebrew/share/guile/site/3.0/
guile_extensions = /opt/homebrew/lib/guile/3.0/extensions/

pg_driver_includes = -I`pg_config --includedir`  \
                     -I./pq_driver.h
pg_driver_libs     = -L`pg_config --libdir` -lpq \
                     -L. -lpq_driver

stampede: stampede.scm libguile-stampede.so
	cp stampede.scm $(guile_site)
	cp libguile-stampede.so $(guile_extensions)

libguile-stampede.so: stampede-extension.o libpq_driver.a
	gcc -o libguile-stampede.so stampede-extension.o \
        `pkg-config --libs guile-3.0`                \
        $(pg_driver_libs)                            \
        -fPIC -shared

stampede-extension.o: stampede-extension.c
	gcc -c stampede-extension.c         \
        `pkg-config --cflags guile-3.0` \
        $(pg_driver_includes)           \
        -fPIC

# maybe I should make this shared as well...
libpq_driver.a: pq_driver.o
	ar -qv libpq_driver.a pq_driver.o

pq_driver.o: pq_driver.c pq_driver.h
	gcc -c pq_driver.c        \
        $(pg_driver_includes) \
        -fPIC

clean:
	rm *.o *.so *.a
