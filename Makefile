CFLAGS = -O2 -Wall -g -D_FILE_OFFSET_BITS=64
SOURCES = src/main.c \
		src/process.c \
		src/thread.c \
		src/time.c \
		src/utils.c \
		src/output.c \
		src/thread_warmup.c \
		src/thread_semaphore.c \
		src/process_semaphore.c
OBJECTS = $(SOURCES:.c=.o)
PROJ = benchIO

default:
	@echo "To build $(PROJ), type:"
	@echo "	make clean SYSTEM"
	@echo "where SYSTEM can be one of the following:"
	@echo "generic      $(PROJ) with base stuff"
	@echo "linux        $(PROJ) with Linux-specific features"
	@echo "windows      $(PROJ) for Windows (Cygwin environment)"
	@echo "(SYSTEM_debug will compile very verbose debug messages)"

generic_debug:
	$(MAKE) generic \
		CFLAGS="-DDEBUG $(CFLAGS)"

generic:
	$(MAKE) $(PROJ)

linux_debug:
	$(MAKE) linux \
		CFLAGS="-DDEBUG $(CFLAGS)"

linux:
	$(MAKE) $(PROJ) \
		SOURCES="$(SOURCES) src/linux.c" \
		CFLAGS="-DLINUX $(CFLAGS)"
	strip $(PROJ)

windows_debug:
	$(MAKE) windows \
		CFLAGS="-DDEBUG $(CFLAGS)"

windows:
	$(MAKE) $(PROJ) \
		CFLAGS="-DWINDOWS $(CFLAGS)"

clean:
	rm -f $(PROJ) src/*.o *~ src/*~ src/header/*~

$(PROJ): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

$(OBJECTS):
