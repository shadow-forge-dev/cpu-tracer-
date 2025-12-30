CC = gcc
CFLAGS = -std=c17 -O3 -ffast-math -pthread -Wall -Wextra -Wno-unused-function -I.
LDFLAGS = -lm -pthread

SRCS = main.c vec3.c mat3.c camera.c bvh.c material.c light.c scene.c sampler.c renderer.c
OBJS = $(SRCS:.c=.o)
TARGET = pathtracer

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.png *.hdr
