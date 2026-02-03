#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 500
// #define DELAY_MS 100

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void blinky_task(struct gpio_dt_spec *led, int delay)
{
    gpio_pin_configure_dt(led, GPIO_OUTPUT_ACTIVE);

    for (;;)
    {
        gpio_pin_toggle_dt(led);
        k_msleep(delay);
    }
}

K_THREAD_DEFINE(blinky0, STACK_SIZE, blinky_task, &led0, 100, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky1, STACK_SIZE, blinky_task, &led1, 200, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky2, STACK_SIZE, blinky_task, &led2, 300, NULL, 5, 0, 0);
K_THREAD_DEFINE(blinky3, STACK_SIZE, blinky_task, &led3, 500, NULL, 5, 0, 0);

// K_THREAD_DEFINE( ... )
