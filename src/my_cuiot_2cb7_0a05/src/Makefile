#.o 文件存放路径
SRC_BIN=lte_dial
#以下两个目录需要继续优化
objects=main.o uart.o lte_dial.o uci_file.o process_fifo_msg.o


$(SRC_BIN):$(objects)
	$(CC) $(CFLAGS) -o $(SRC_BIN)  $(objects) -luci 
	
%.o:%.c
	$(CC) $(CFLAGS)  -c $< -o $@ 

.PHONY: clean

clean:
	rm *.o
	rm $(SRC_BIN)
	