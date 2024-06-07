
# need to copy over products to site locations...
stampede: stampede.scm libguile-stampede.so
	cp stampede.scm /opt/homebrew/share/guile/site/3.0/
	cp libguile-stampede.so /opt/homebrew/lib/guile/3.0/extensions/

libguile-stampede.so: stampede-extension.o libpq_driver.a
	gcc -o libguile-stampede.so stampede-extension.o \
        `pkg-config --libs guile-3.0`                \
        -L`pg_config --libdir` -lpq                  \
        -L. -lpq_driver                              \
        -fPIC -shared

stampede-extension.o: stampede-extension.c
	gcc -c stampede-extension.c         \
        `pkg-config --cflags guile-3.0` \
        -I`pg_config --includedir`      \
        -I./pq_driver.h                 \
        -fPIC

# maybe I should make this shared as well...
libpq_driver.a: pq_driver.o
	ar -qv libpq_driver.a pq_driver.o

pq_driver.o: pq_driver.c pq_driver.h
	gcc -c pq_driver.c             \
        -I`pg_config --includedir` \
        -I./pq_driver.h            \
        -fPIC
