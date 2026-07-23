// encoder_app.c

#include "encoder.h"
#include "ti_msp_dl_config.h" // 包含您的 Sysconfig 生成的头文件
#include "log.h"
#include "encoder_user.h"

// 定义您的机器人需要的编码器数量
#define NUM_ROBOT_ENCODERS 2 // 四个编码器

// 存储编码器实例数据的数组
encoder_instance_t robot_encoders[NUM_ROBOT_ENCODERS];

// 编码器管理器实例
encoder_manager_t robot_encoder_manager = {
    .encoders = robot_encoders,
    .num_encoders = NUM_ROBOT_ENCODERS,
    .gpio_read_func = NULL,         // 在 init 中设置
    .attach_interrupt_func = NULL,  // 在 init 中设置
    .enter_critical_func = NULL,    // 在 init 中设置
    .exit_critical_func = NULL,     // 在 init 中设置
};

#define ENCODER_PORTB_MAP_OFFSET 64U
#define ENCODER_PORTA_MAP(iidx) ((uint16_t) (iidx))
#define ENCODER_PORTB_MAP(iidx) \
    ((uint16_t) (ENCODER_PORTB_MAP_OFFSET + (uint16_t) (iidx)))
#define MAX_GPIO_MAP_ENTRIES 128U

static encoder_instance_t* interrupt_map_to_encoder_instance[MAX_GPIO_MAP_ENTRIES];

static const encoder_pin_config_t encoder_pins[] = {
    {PORTB_PORT, PORTB_M1_ENCODER_A_PIN,
        ENCODER_PORTB_MAP(PORTB_M1_ENCODER_A_IIDX)},
    {PORTB_PORT, PORTB_M1_ENCODER_B_PIN,
        ENCODER_PORTB_MAP(PORTB_M1_ENCODER_B_IIDX)},
    {PORTA_PORT, PORTA_M2_ENCODER_A_PIN,
        ENCODER_PORTA_MAP(PORTA_M2_ENCODER_A_IIDX)},
    {PORTB_PORT, PORTB_M2_ENCODER_B_PIN,
        ENCODER_PORTB_MAP(PORTB_M2_ENCODER_B_IIDX)},
};

#define NUM_ENCODER_PINS (sizeof(encoder_pins) / sizeof(encoder_pins[0]))

// GPIO 读取函数
uint8_t mspm0_gpio_read(void *gpio_handle, uint32_t pin_mask) {
    GPIO_Regs* gpio_regs = (GPIO_Regs*)gpio_handle;
    return DL_GPIO_readPins(gpio_regs, pin_mask) ? 1 : 0;
}

// 中断挂载函数
bool mspm0_attach_interrupt(void *pin_handle, void (*isr_handler)(void *arg), void *arg) {
    uint16_t map_index = (uint16_t)(uintptr_t)pin_handle;
    (void) isr_handler;
    
    if (map_index < MAX_GPIO_MAP_ENTRIES) {
        interrupt_map_to_encoder_instance[map_index] = (encoder_instance_t*)arg;
        return true; 
    } else {
        log_e("Invalid pin handle index (IIDX) for interrupt attachment: %u", map_index);
        return false; 
    }
}

// 简化的 GPIO 中断服务程序
void encoder_group1_irq_handler(void)
{       
		// 遍历所有编码器引脚，处理中断
		for (uint8_t i = 0; i < NUM_ENCODER_PINS; i++) {
				if (DL_GPIO_getEnabledInterruptStatus(encoder_pins[i].port,
                        encoder_pins[i].pin_mask)) {
						// 清除对应引脚的中断标志
						DL_GPIO_clearInterruptStatus(encoder_pins[i].port,
                            encoder_pins[i].pin_mask);
						
						// 获取对应的编码器实例
						uint16_t map_index = encoder_pins[i].map_index;
						if (map_index < MAX_GPIO_MAP_ENTRIES &&
								interrupt_map_to_encoder_instance[map_index] != NULL) {
								encoder_update(interrupt_map_to_encoder_instance[map_index]);
						}
				}
		}   
}


// 编码器应用程序初始化
void encoder_application_init(void) {
    // 定义编码器配置数组
    encoder_config_t encoder_configs[NUM_ROBOT_ENCODERS] = {
        // Encoder 0 / motor C (A=PB13, B=PB12)
        {
            .pin1_gpio_handle = PORTB_PORT,
            .pin1_bitmask = PORTB_M1_ENCODER_A_PIN,
            .pin1_handle = (void*)(uintptr_t)
                ENCODER_PORTB_MAP(PORTB_M1_ENCODER_A_IIDX),
            .pin2_gpio_handle = PORTB_PORT, 
            .pin2_bitmask = PORTB_M1_ENCODER_B_PIN,
            .pin2_handle = (void*)(uintptr_t)
                ENCODER_PORTB_MAP(PORTB_M1_ENCODER_B_IIDX)
        },
        // Encoder 1 / motor D (A=PA29, B=PB26)
        {
            .pin1_gpio_handle = PORTA_PORT,
            .pin1_bitmask = PORTA_M2_ENCODER_A_PIN,
            .pin1_handle = (void*)(uintptr_t)
                ENCODER_PORTA_MAP(PORTA_M2_ENCODER_A_IIDX),
            .pin2_gpio_handle = PORTB_PORT,
            .pin2_bitmask = PORTB_M2_ENCODER_B_PIN,
            .pin2_handle = (void*)(uintptr_t)
                ENCODER_PORTB_MAP(PORTB_M2_ENCODER_B_IIDX)
        },
        // Encoder 2
//        {
//            .pin1_gpio_handle = PORTB_PORT, 
//            .pin1_bitmask = PORTB_ENCODER_6_PIN,
//            .pin1_handle = (void*)(uintptr_t)PORTB_ENCODER_6_IIDX,
//            .pin2_gpio_handle = PORTB_PORT, 
//            .pin2_bitmask = PORTB_ENCODER_5_PIN,
//            .pin2_handle = (void*)(uintptr_t)PORTB_ENCODER_5_IIDX
//        },
//        // Encoder 3
//        {
//            .pin1_gpio_handle = PORTB_PORT, 
//            .pin1_bitmask = PORTB_ENCODER_8_PIN,
//            .pin1_handle = (void*)(uintptr_t)PORTB_ENCODER_8_IIDX,
//            .pin2_gpio_handle = PORTB_PORT, 
//            .pin2_bitmask = PORTB_ENCODER_7_PIN,
//            .pin2_handle = (void*)(uintptr_t)PORTB_ENCODER_7_IIDX
//        },
    };
    
    // 初始化编码器管理器
    encoder_manager_init(
        &robot_encoder_manager,
        encoder_configs, 
        NUM_ROBOT_ENCODERS,
        mspm0_gpio_read,         // GPIO 读取函数
        mspm0_attach_interrupt,  // 中断挂载函数
        NULL, 
        NULL  
    );
    
    // 启用相关的 GPIO 中断向量
    NVIC_EnableIRQ(PORTA_INT_IRQN);
    NVIC_EnableIRQ(PORTB_INT_IRQN);
}
