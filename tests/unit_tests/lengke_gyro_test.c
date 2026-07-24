#include "tests.h"

#include "delay.h"
#include "lengke_gyro.h"
#include "log.h"
#include "log_config.h"

void lengke_gyro_test(void)
{
    lengke_gyro_init();

    for (;;) {
        log_i("lengke gyro: z_rate=%.2f dps, yaw=%.2f deg",
            lengke_gyro_get_z_rate(), lengke_gyro_get_yaw());
        delay_ms(200U);
    }
}
