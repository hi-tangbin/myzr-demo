
#include <stdio.h>
#include <unistd.h>
#include <my_sdk.h>

void my_gpio_callback(char *interface, int event)
{
	printf("GPIO: %s %d\n", interface, event);
}

void my_uart_callback(char *interface, char *buffer)
{
	printf("UART: %s %s\n", interface, buffer);
}

int main(int argc, char **argv)
{
	char buffer[512];
	unsigned char c;

#if 1
	void *uart = my_uart_init(my_uart_callback);
	if (uart == NULL)
		return -1;
#endif

	//my_serial_send(index, buffer);

#if 1
	void *gpio = my_gpio_init(my_gpio_callback);
	if (gpio == NULL)
		return -1;
#endif

#if 0
	my_gps_init();
	my_gps_read(1, buffer);
#endif

	for (;;) {
		c++;
		my_gpio_set_val(110, c%2);
		printf("MY_SDK_DEMO: gpio110 out %d\n", c%2);
		sleep(60);
	}
}
