#include <stdint.h>
#include "../libopeninv/include/params.h"
#ifndef TEST_TMOCK_H
#define TEST_TMOCK_H

#define TIM_SR_CC2IF			  (1 << 2)
#define PERIPH_BASE			    (0x40000000U)
#define PERIPH_BASE_APB1		(PERIPH_BASE + 0x00000)
#define TIM3_BASE			      (PERIPH_BASE_APB1 + 0x0400)
#define TIM3				        TIM3_BASE

enum tim_ic_id {
	TIM_IC1,
	TIM_IC2,
	TIM_IC3,
	TIM_IC4,
};

uint32_t rtc_get_counter_val();
bool timer_get_flag(uint32_t timer_peripheral, uint32_t flag);
float timer_get_ic_value(uint32_t timer_peripheral, enum tim_ic_id ic_id);
#endif
