#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#define BUTTON_PIN 20
#define BUTTON_NODE DT_NODELABEL(gpio0)

const struct device *gpio_dev;

// Wait for Press (Active Low) ---
//wait for the pin to go to 0 (Ground)
void wait_for_press(void) {
    while(gpio_pin_get(gpio_dev, BUTTON_PIN) == 1) { // Wait while High (Idle)
        k_msleep(10);
    }
    k_msleep(50); // Debounce
}

// --- HELPER 2: Wait for Release (Active Low) ---
// We wait for the pin to go back to 1 (3.3V)
void wait_for_release(void) {
    while(gpio_pin_get(gpio_dev, BUTTON_PIN) == 0) { // Wait while Low (Held)
        k_msleep(10);
    }
    k_msleep(50); // Debounce
}

int main(void)
{
    gpio_dev = DEVICE_DT_GET(BUTTON_NODE);
    int64_t start, stop, diff;

    if (!device_is_ready(gpio_dev)) return 0;

    // Use PULL_UP so it stays at 3.3V when not pressed
    gpio_pin_configure(gpio_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);

    printf("\n=== 3-Second Game ===\n");
    printf("Instructions: Press GP20 to START. Press again to STOP.\n");

    while (1) {
        printf("\nWaiting for START press...\n");

        // 1. Wait for user to push button (Logic 0)
        wait_for_press();
        
        start = k_uptime_get(); 
        printf("Timer STARTED! Press again in 3s...\n");
        
        // Wait for user to let go (Logic 1)
        wait_for_release(); 

        // 2. Wait for user to push again
        wait_for_press();
        
        stop = k_uptime_get();
        wait_for_release(); 

        // 3. Results
        diff = stop - start;
        printf("Stopped at: %lld ms\n", diff);

        if (diff >= 2700 && diff <= 3300) {
            printf("RESULT: PERFECT! (Within 10%%)\n");
        } else {
            printf("RESULT: Too far off. Try again.\n");
        }
        
        k_msleep(1000); 
    }
    return 0;
}