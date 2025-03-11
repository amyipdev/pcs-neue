# SPDX-License-Identifier: GPL-2.0-or-later

CC := cc
CFLAGS += -O3 -Wall -Werror -std=gnu2x $(shell pkg-config --cflags glib-2.0)
LIBS += $(shell pkg-config --libs glib-2.0)
SOURCEDIR := src
SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
TARGETS := $(SOURCES:.c=.o)
HEADERS := $(shell find $(SOURCEDIR) -name '*.h')
TARGET := pcsbench

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(TARGETS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS)

clean:
	rm -f $(TARGETS) pcsbench result
