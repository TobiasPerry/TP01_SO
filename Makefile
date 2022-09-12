SRCAPP := aplicacion.c
SRCVIEW := vista.c

RM := rm --force
MAKE := $(MAKE) --no-print-directory

CC := gcc
CFLAGS := -Wall -lrt -lm -pthread
DBGFLAGS := -g
all:app vista

app: aplicacion.c
	$(CC) aplicacion.c $(CFLAGS) -o aplicacion
vista: vista.c
	$(CC) vista.c $(CFLAGS) -o vista

dbg:
	$(CC) vista.c $(CFLAGS) $(DBGFLAGS) -o vista
	$(CC) aplicacion.c $(CFLAGS) $(DBGFLAGS) -o aplicacion
clean:
	$(RM) md5
	$(RM) vista
	$(RM) Resultado.txt

re:
	$(MAKE) clean
	$(MAKE) all

.PHONY: clean re dbg