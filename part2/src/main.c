#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 500
#define PRIO_BUTTON_TASK 5

/* 1. Synchronization Primitives */
K_SEM_DEFINE(button_sem, 0, 1); // Signal from ISR to button_task
K_MUTEX_DEFINE(led_mutex);      // Protects led_index

/* 2. Hardware Specs */
static int led_index = 0;
static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios)};
static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(DT_ALIAS(btn), gpios);

/* 3. Button ISR (Deferred handling) */
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // Give semaphore to wake up the button_task
    k_sem_give(&button_sem);
}

/* 4. Button Task */
void button_task(void *p1, void *p2, void *p3)
{
    static struct gpio_callback button_cb_data;

    gpio_pin_configure_dt(&btn, GPIO_INPUT);
    gpio_init_callback(&button_cb_data, button_isr, BIT(btn.pin));
    gpio_add_callback(btn.port, &button_cb_data);
    gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);

    while (1)
    {
        k_sem_take(&button_sem, K_FOREVER);

        // Lock mutex to update shared index safely
        k_mutex_lock(&led_mutex, K_FOREVER);
        led_index = (led_index + 1) % 4;
        k_mutex_unlock(&led_mutex);

        k_msleep(100); // Debounce
    }
}

/* 5. Blinky Task */
void blinky_task(void *p1, void *p2, void *p3)
{
    for (int i = 0; i < 4; i++)
    {
        gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
    }

    while (1)
    {
        int current;

        // Lock mutex to read shared index safely
        k_mutex_lock(&led_mutex, K_FOREVER);
        current = led_index;
        k_mutex_unlock(&led_mutex);

        gpio_pin_toggle_dt(&leds[current]);
        k_msleep(200);
    }
}

/* 6. Thread Definitions */
K_THREAD_DEFINE(blink_tid, STACK_SIZE, blinky_task, NULL, NULL, NULL, PRIO_BUTTON_TASK, 0, 0);
K_THREAD_DEFINE(button_tid, STACK_SIZE, button_task, NULL, NULL, NULL, PRIO_BUTTON_TASK, 0, 0);