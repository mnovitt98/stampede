
guile-with-stampede: guile-with-stampede.o libpq_driver.a
	gcc -o guile guile-with-stampede.o `pkg-config --libs guile-3.0` \
        -L`pg_config --libdir` -lpq \
        -L. -lpq_driver

guile-with-stampede.o: guile-with-stampede.c 
	gcc -c guile-with-stampede.c `pkg-config --cflags guile-3.0` \
        -I`pg_config --includedir` \
        -I./pq_driver.h

libpq_driver.a: pq_driver.o
	ar -qv libpq_driver.a pq_driver.o

pq_driver.o: pq_driver.c pq_driver.h
	gcc -c pq_driver.c -I`pg_config --includedir` \
                       -I./pq_driver.h \
                       -fPIC
