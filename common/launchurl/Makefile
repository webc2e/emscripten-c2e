# To build the executable
#	make
# To build a shared library
# 	make launchurl.a
# To link to the shared library, add launchurl.a to the list of objects
# when linking.

all: launchurl.a launchurl

%.o: %.c
	gcc -c -Wall -o $@ $<

SOURCE := launchurl.c try_kde.c try_browservar.c try_misc.c try_gnome.c 
OBJ := $(patsubst %.c,%.o,$(SOURCE))

launchurl.a: $(OBJ)
	ar rc launchurl.a $(OBJ)

launchurl: main.c launchurl.a
	gcc -o launchurl -Wall main.c -ldl launchurl.a

clean:
	@rm -f *.a
	@rm -f *.o
	@rm -f launchurl

