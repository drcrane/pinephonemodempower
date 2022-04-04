#include <gpiod.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

struct gpiod_line * modem_ctrl[6];

#define PIN_DTR 34
#define PIN_PWRKEY 35
#define PIN_RESET 68
#define PIN_APREADY 231
#define PIN_DISABLE 232
#define PIN_STATUS 233

#define LINE_DTR 0
#define LINE_PWRKEY 1
#define LINE_RESET 2
#define LINE_APREADY 3
#define LINE_DISABLE 4
#define LINE_STATUS 5

static struct gpiod_line * gpio_get_output_line(struct gpiod_chip * gpio_chip, unsigned int pin) {
	struct gpiod_line * gpio_line;
	int rc;
	gpio_line = gpiod_chip_get_line(gpio_chip, pin);
	if (!gpio_line) { return NULL; }
	rc = gpiod_line_request_output(gpio_line, "minimanager", 0);
	if (rc < 0) { gpiod_line_release(gpio_line); return NULL; }
	fprintf(stdout, "pin %u OK\n", pin);
	return gpio_line;
}

static struct gpiod_line * gpio_get_input_line(struct gpiod_chip * gpio_chip, unsigned int pin) {
	struct gpiod_line * gpio_line;
	int rc;
	gpio_line = gpiod_chip_get_line(gpio_chip, pin);
	if (!gpio_line) { return NULL; }
	rc = gpiod_line_request_input(gpio_line, "minimanager");
	if (rc < 0) { gpiod_line_release(gpio_line); return NULL; }
	return gpio_line;
}

void pinephonemodem_open(struct gpiod_chip * gpio_chip) {
	modem_ctrl[LINE_DTR] = gpio_get_output_line(gpio_chip, PIN_DTR);
	modem_ctrl[LINE_PWRKEY] = gpio_get_output_line(gpio_chip, PIN_PWRKEY);
	modem_ctrl[LINE_RESET] = gpio_get_output_line(gpio_chip, PIN_RESET);
	modem_ctrl[LINE_APREADY] = gpio_get_output_line(gpio_chip, PIN_APREADY);
	modem_ctrl[LINE_DISABLE] = gpio_get_output_line(gpio_chip, PIN_DISABLE);
	modem_ctrl[LINE_STATUS] = gpio_get_input_line(gpio_chip, PIN_STATUS);
}

void pinephonemodem_close() {
	size_t i;
	for (i = 0; i < 6; ++i) {
		gpiod_line_release(modem_ctrl[i]);
	}
}

void pinephonemodem_dumpstatus() {
	size_t i;
	for (i = 0; i < 6; ++i) {
		if (modem_ctrl[i]) {
			fprintf(stdout, "%d : %d\n", (int)i, gpiod_line_get_value(modem_ctrl[i]));
		}
	}
}

int main(int argc, char *argv[]) {
	struct gpiod_line *gpio_line;
	struct gpiod_chip *gpio_chip;
	int res = 0;
	int rc;
	struct timespec ts;

	gpio_chip = gpiod_chip_open_by_name("gpiochip1");
	if (gpio_chip == NULL) {
		return 1;
	}

	pinephonemodem_open(gpio_chip);

	pinephonemodem_dumpstatus();

	gpiod_line_set_value(modem_ctrl[LINE_PWRKEY], 1);
	sleep(1);
	gpiod_line_set_value(modem_ctrl[LINE_PWRKEY], 0);

	//sleep(1);
	gpiod_line_set_value(modem_ctrl[LINE_RESET], 1);
	sleep(1);
	gpiod_line_set_value(modem_ctrl[LINE_RESET], 0);

	pinephonemodem_dumpstatus();
	
	rc = 1;
	while (rc == 1) {
		rc = gpiod_line_get_value(modem_ctrl[LINE_STATUS]);
		fprintf(stdout, "STATUS: %d\n", rc);
		ts.tv_sec = 0;
		ts.tv_nsec = 500000000;
		nanosleep(&ts, NULL);
	}

	//sleep(500);

	pinephonemodem_close();
quit_close_chip:
	gpiod_chip_close(gpio_chip);
	return res;
}

