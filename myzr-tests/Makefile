
CC		= ${CROSS_COMPILE}gcc
STRIP   = ${CROSS_COMPILE}strip
RM      = rm

CUR_SOURCE	= ${wildcard *.c}
CUR_OBJS	= ${patsubst %.c, %.out, $(CUR_SOURCE)}

all: $(CUR_OBJS)
	$(STRIP) $(CUR_OBJS)

$(CUR_OBJS):%.out:%.c
	$(CC) $^ -o $@

clean:
	$(RM) *.out
