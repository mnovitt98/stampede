
guile-with-stampede: guile-with-stampede.o
	gcc -o guile guile-with-stampede.o `pkg-config --libs  guile-3.0` -L`pg_config --libdir` -lpq

guile-with-stampede.o: guile-with-stampede.c
	gcc -c guile-with-stampede.c `pkg-config --cflags guile-3.0` -I`pg_config --includedir`
