/*
 * stepper.c
 * TB6560 Stepper Motor Driver for STM32F401CDU6
 *
 * Uses TIM2 CH1 (PA0) for hardware PWM pulse generation.
 * TIM2 Update Interrupt counts pulses and stops at target.
 *
 * Timer Config (from CubeMX):
 *   - PSC  = 83   -> 84MHz / 84 = 1MHz tick
 *   - ARR  = 499  -> 1MHz / 500 = 2kHz step rate
 *   - CCR1 = 250  -> 50% duty cycle
 *
 * At 2kHz step rate with 3200 steps/rev:
 *   Full rotation = 3200 / 2000 = 1.6 seconds
 */

#include "stepper.h"

#include "config.h"



/* Private handle */
static Stepper_Handle_t stepper1, stepper2;

/* ---------- GPIO helpers ---------- */

static void Stepper_SetDirection(Motor_ID M, uint8_t dir)
{

	switch (M) {
		case LegMotor_1:
		    if (dir == STEPPER_CW)
		    {
		        HAL_GPIO_WritePin(M1_CWP_Port, M1_CWP_Pin, GPIO_PIN_SET);    /* CW+ HIGH */
		    }
		    else
		    {
		        HAL_GPIO_WritePin(M1_CWP_Port, M1_CWP_Pin, GPIO_PIN_RESET);  /* CW+ LOW */
		    }
			break;
		case LegMotor_2:
		    if (dir == STEPPER_CW)
		    {
		        HAL_GPIO_WritePin(M2_CWP_Port, M2_CWP_Pin, GPIO_PIN_SET);    /* CW+ HIGH */
		    }
		    else
		    {
		        HAL_GPIO_WritePin(M2_CWP_Port, M2_CWP_Pin, GPIO_PIN_RESET);  /* CW+ LOW */
		    }
			break;
		default:
			break;
	}


}

static void Stepper_SetEnable(Motor_ID M, uint8_t enable)
{
	switch (M) {
		case LegMotor_1:
		    if (enable)
		    {
		        HAL_GPIO_WritePin(M1_ENP_Port, M1_ENP_Pin, GPIO_PIN_RESET);  /* EN+ LOW = enabled (active low) */
		    }
		    else
		    {
		        HAL_GPIO_WritePin(M1_ENP_Port, M1_ENP_Pin, GPIO_PIN_SET);    /* EN+ HIGH = disabled */
		    }
			break;
		case LegMotor_2:
		    if (enable)
		    {
		        HAL_GPIO_WritePin(M2_ENP_Port, M2_ENP_Pin, GPIO_PIN_RESET);  /* EN+ LOW = enabled (active low) */
		    }
		    else
		    {
		        HAL_GPIO_WritePin(M2_ENP_Port, M2_ENP_Pin, GPIO_PIN_SET);    /* EN+ HIGH = disabled */
		    }
			break;
		default:
			break;
	}
}

/* ---------- Public functions ---------- */

void Stepper_Init(Motor_ID M)
{

	switch(M){
		case LegMotor_1:
		    stepper1.target_steps  = 0;
		    stepper1.current_steps = 0;
		    stepper1.state         = STEPPER_IDLE;
		    stepper1.direction     = STEPPER_CW;

		    HAL_GPIO_WritePin(M1_CLKN_Port, M1_CLKN_Pin,  GPIO_PIN_RESET);  /* CLK- LOW */
		    HAL_GPIO_WritePin(M1_CWN_Port, M1_CWN_Pin, GPIO_PIN_RESET);  /* CW-  LOW */
		    HAL_GPIO_WritePin(M1_ENN_Port, M1_ENN_Pin, GPIO_PIN_RESET);  /* EN-  LOW */
		    /* Disable motor by default */
		    Stepper_Disable(LegMotor_1);
			break;
		case LegMotor_2:
		    stepper2.target_steps  = 0;
		    stepper2.current_steps = 0;
		    stepper2.state         = STEPPER_IDLE;
		    stepper2.direction     = STEPPER_CW;

		    HAL_GPIO_WritePin(M2_CLKN_Port, M2_CLKN_Pin,  GPIO_PIN_RESET);  /* CLK- LOW */
		    HAL_GPIO_WritePin(M2_CWN_Port, M2_CWN_Pin, GPIO_PIN_RESET);  /* CW-  LOW */
		    HAL_GPIO_WritePin(M2_ENN_Port, M2_ENN_Pin, GPIO_PIN_RESET);  /* EN-  LOW */
		    /* Disable motor by default */
		    Stepper_Disable(LegMotor_2);
			break;
		case LegMotor_3:

			break;
		default:
			break;
	}


    /* Set fixed-low pins */







}

void Stepper_Move(Motor_ID M, int32_t steps)
{
	switch(M){
	case LegMotor_1:
	    if (steps == 0) return;
	    if (stepper1.state == STEPPER_RUNNING) return;

	    /* Set direction based on sign */
	    if (steps > 0)
	    {
	        stepper1.direction = STEPPER_CW;
	        stepper1.target_steps = (uint32_t)steps;
	    }
	    else
	    {
	        stepper1.direction = STEPPER_CCW;
	        stepper1.target_steps = (uint32_t)(-steps);
	    }

	    stepper1.current_steps = 0;
	    stepper1.state = STEPPER_RUNNING;

	    /* Set direction pin */
	    Stepper_SetDirection(LegMotor_1, stepper1.direction);

	    /* Enable driver */
	    Stepper_SetEnable(LegMotor_1, 1);

	    /* Small delay for driver to settle */
	//    HAL_Delay(1);

	    /* Start PWM on TIM2 CH1 (PA0) - no MOE needed, TIM2 is general purpose */
	    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	    /* Enable TIM2 update interrupt to count pulses */
	    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

		break;
	case LegMotor_2:
	    if (steps == 0) return;
	    if (stepper2.state == STEPPER_RUNNING) return;

	    /* Set direction based on sign */
	    if (steps > 0)
	    {
	        stepper2.direction = STEPPER_CW;
	        stepper2.target_steps = (uint32_t)steps;
	    }
	    else
	    {
	        stepper2.direction = STEPPER_CCW;
	        stepper2.target_steps = (uint32_t)(-steps);
	    }

	    stepper2.current_steps = 0;
	    stepper2.state = STEPPER_RUNNING;

	    /* Set direction pin */
	    Stepper_SetDirection(LegMotor_2, stepper2.direction);

	    /* Enable driver */
	    Stepper_SetEnable(LegMotor_2, 1);
	    /* Small delay for driver to settle */
	//    HAL_Delay(1);

	    /* Start PWM on TIM2 CH1 (PA0) - no MOE needed, TIM2 is general purpose */
	    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	    /* Enable TIM2 update interrupt to count pulses */
	    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);

		break;
	default:
		break;
	}


}

void Stepper_Stop(Motor_ID M)
{
	switch(M){
		case LegMotor_1:
			HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
			__HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
			stepper1.current_steps = 0;
			stepper1.target_steps  = 0;
			stepper1.state = STEPPER_IDLE;
			break;
		case LegMotor_2:
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
			__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
			stepper2.current_steps = 0;
			stepper2.target_steps  = 0;
			stepper2.state = STEPPER_IDLE;
			break;
		default:
			break;
		}


}

void Stepper_Enable(Motor_ID M)
{
	Stepper_SetEnable(M, 1);
}

void Stepper_Disable(Motor_ID M)
{
	Stepper_SetEnable(M, 0);
}

uint8_t Stepper_IsBusy(Motor_ID M)
{
	switch(M){
		case LegMotor_1:
			return (stepper1.state == STEPPER_RUNNING) ? 1 : 0;
			break;
		case LegMotor_2:
			return (stepper2.state == STEPPER_RUNNING) ? 1 : 0;
			break;
		default:
			break;
		}
}

/* ---------- IRQ Handler ---------- */

void Stepper_IRQ_Handler(Motor_ID M)
{
	switch(M){
		case LegMotor_1:
		    if (stepper1.state != STEPPER_RUNNING) return;

		    stepper1.current_steps++;

		    if (stepper1.current_steps >= stepper1.target_steps)
		    {
		        Stepper_Stop(LegMotor_1);
		    }
			break;
		case LegMotor_2:
		    if (stepper2.state != STEPPER_RUNNING) return;

		    stepper2.current_steps++;

		    if (stepper2.current_steps >= stepper2.target_steps)
		    {
		        Stepper_Stop(LegMotor_2);
		    }
			break;
		default:
			break;
		}

}



