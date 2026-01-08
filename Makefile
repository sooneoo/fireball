CC=gcc


CACHE = .cache
RELEASE = $(CACHE)/release
TARGET = fireball

CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -O2 -g
CFLAGS += -Isrc/include/
CFLAGS += $$(pkg-config --cflags raylib)

LIBS += -lm $$(pkg-config --libs raylib)


OBJS += $(CACHE)/main.o


all: build


build: env $(RELEASE)/$(TARGET)


$(RELEASE)/$(TARGET): $(OBJS)
	$(CC)  $(OBJS) $(LIBS) -o $(RELEASE)/$(TARGET)


$(CACHE)/main.o: src/fireball/main.c
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: exec clean env


exec: build
	$(RELEASE)/$(TARGET)


clean:
	rm -rvf $(CACHE)


env: 
	mkdir -pv $(CACHE)
	mkdir -pv $(RELEASE)


