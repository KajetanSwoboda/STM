/**
  ******************************************************************************
  * @file    pid_controller.h
  * @author  AW           Adrian.Wojcik@put.poznan.pl
  * @version 1.0
  * @date    06 Sep 2021
  * @brief   Simple PID controller implementation.
  *          Header file.
  *
  ******************************************************************************
  */

/* Config --------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/

/* Typedef -------------------------------------------------------------------*/
typedef struct {
  float Kp;        // Proportional gain
  float Ki;        // Integral gain
  float Kd;        // Derivative gain
  float N;         // Derivative filter constant
  float Ts;        // Sample time [s]
  float e_prev;    // Previous input (control error)
  float e_int;     // Input integral
  float d_prev;    // Previous derivative
  float LimitUpper;     // Output upper saturation
  float LimitLower;     // Output lower saturation
} PID_HandleTypeDef;

/* Define --------------------------------------------------------------------*/

/* Macro ---------------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/**
 * @brief PID controller (re)initialization: sets zero initial conditions.
 * @param[in] hpid : PID controller handler
 */
void PID_Init(PID_HandleTypeDef* hpid);

/**
 * @brief PID controller response
 * @param[in] hpid : PID controller handler
 * @param[in] yref : Reference signal (set point)
 * @param[in] y    : Measurement signal
 * @return Controller output signal (control response)
 */
float PID_GetOutput(PID_HandleTypeDef* hpid, float yref, float y);
