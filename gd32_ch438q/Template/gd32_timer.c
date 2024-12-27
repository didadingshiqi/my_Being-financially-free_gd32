#include "gd32_timer.h"

static __IO uint32_t timer_val = 0;

void TIM_Init(void)
{
    timer_parameter_struct tim_struct = {0};
    
    rcu_periph_clock_enable(RCU_TIMER1);
    
    tim_struct.counterdirection = TIMER_COUNTER_UP;  // 向上计数
    tim_struct.prescaler = (108 - 1);  // 预分频：108MHz / 108 = 1MHz
    tim_struct.period = (1000 - 1);  // 周期：1000 / 1MHz = 1ms
    
    timer_init(TIMER1, &tim_struct);
    
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    nvic_irq_enable(TIMER1_IRQn, 5, 0);
    
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
		
		timer_disable(TIMER1);  // 停止定时器1
    //timer_enable(TIMER1);
}

void TIM_DelayMs(__IO uint32_t ms)
{
	timer_enable(TIMER1);
  timer_val = ms;
  while(timer_val);
	timer_disable(TIMER1);  // 停止定时器1
	
}

void TIM_ValDecrese(void)
{
    if(timer_val) --timer_val;
}
