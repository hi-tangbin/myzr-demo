
# 获取当前目录下的 c 文件集，放在变量 SRC_LIST 中
SRC_LIST	= ${wildcard *.c}

# 将对应的 c 文件名转为 out 文件后放在下面的 OBJ_LIST 变量中
OBJ_LIST	= ${patsubst %.c, %.out, $(SRC_LIST)}

all: $(OBJ_LIST)

$(OBJ_LIST):%.out:%.c
	$(CC) $(CFLAGS) -c $^ -o $(BIN_DIR)/$@
