#include "tests.h"
#include "delay.h"
#include "gray_detection.h"
#include "log.h"

static uint8_t gray_datas[TRACK_SENSOR_COUNT];

void gd_test(void) {
		gray_detection_init();
    for ( ; ; )  {
				uint16_t temp_data = gray_read_byte();
			   for (int i = 0; i < TRACK_SENSOR_COUNT; i++) {
					gray_datas[i] = (temp_data >> i) & 0x01; // 提取每一位并存入数组
				}
				log_i("Gray Data: %d %d %d %d %d %d %d %d",
						gray_datas[0], gray_datas[1], gray_datas[2], gray_datas[3],
						gray_datas[4], gray_datas[5], gray_datas[6], gray_datas[7]);
				delay_ms(200);
    }
}
