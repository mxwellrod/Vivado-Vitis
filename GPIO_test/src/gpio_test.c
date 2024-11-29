/*
*   
*   This simple program test GPIO on ZyboZ7 ussing buttons and switches to interact with LEDs, all through EMIO
*
*/


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Xilinx includes. */
#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_io.h" 
#include "xil_mmu.h"
#include "xuartps.h"

/*---------------------- GPIO Parameters---------------------*/
#define LED_CHANNEL 1
#define SW_CHANNEL 2
#define BTN_CHANNEL 1
#define XGPIO_AXI_LED_BASEADDR XPAR_XGPIO_0_BASEADDR
#define XGPIO_AXI_BTN_BASEADDR XPAR_XGPIO_1_BASEADDR
int LED1 = 0x1; // LED1 mask
/*-----------------------------------------------------------*/


/*---------------------- freeRTOS task parameters------------*/
#define DELAY_100_ms        100UL
#define DELAY_500_ms        500UL
static void checkGPIOs_Task( void *pvParameters );
static TaskHandle_t xThread1;
const TickType_t x100ms = pdMS_TO_TICKS( DELAY_100_ms);
const TickType_t x500ms = pdMS_TO_TICKS( DELAY_500_ms);
/*-----------------------------------------------------------*/


/*---------- GPIO driver instances globally defined ---------*/
XGpio Gpio;
XGpio Gpio_btns;
/*-----------------------------------------------------------*/


int main( void ){

    int status = 0;
    XGpio_Config *CfgPtr;

    xil_printf( "freeRTOS main. OCM demo\r\n" );

    // GPIO config leds
    CfgPtr = XGpio_LookupConfig(XGPIO_AXI_LED_BASEADDR);
    if(CfgPtr == NULL){
        xil_printf("Error looking up the leds gpio config\r\n");
        return XST_FAILURE;
    }

    status = XGpio_CfgInitialize(&Gpio, CfgPtr, XGPIO_AXI_LED_BASEADDR);

    if(status != XST_SUCCESS){
        xil_printf("Config Initialization of leds failed\r\n");
        return XST_FAILURE;
    }
    status = XGpio_Initialize(&Gpio, XGPIO_AXI_LED_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("GPIO Initialization failed\r\n");
        return XST_FAILURE;
    }


    // GPIO config butttons
    CfgPtr = XGpio_LookupConfig(XGPIO_AXI_BTN_BASEADDR);
    if(CfgPtr == NULL){
        xil_printf("Error looking up the buttons gpio config\r\n");
        return XST_FAILURE;
    }
    status = XGpio_CfgInitialize(&Gpio_btns, CfgPtr, XGPIO_AXI_BTN_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("Config initialization of Buttons failed\r\n");
        return XST_FAILURE;
    }

    status = XGpio_Initialize(&Gpio_btns, XGPIO_AXI_BTN_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("GPIO Initialization of Buttons failed\r\n");
        return XST_FAILURE;
    } 

    // GPIO direction settings leds + buttons
    XGpio_SetDataDirection(&Gpio, LED_CHANNEL, 0x00);  // output
    XGpio_SetDataDirection(&Gpio, SW_CHANNEL, 0xFF); // input
    XGpio_SetDataDirection(&Gpio_btns, BTN_CHANNEL, 0xFF); // input

	/* Task Creation */
	xTaskCreate( 	checkGPIOs_Task, 					/* The function that implements the task. */
					( const char * ) "Thread1", 		/* Text name for the task, provided to assist debugging only. */
					configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
					NULL, 						/* The task parameter is not used, so set to NULL. */
					tskIDLE_PRIORITY,			/* The task runs at the idle priority. */
					&xThread1 );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}


/*-----------------------------------------------------------*/
static void checkGPIOs_Task( void *pvParameters )
{
    uint32_t ledValue = 0;
    uint32_t swState = 0;
    uint32_t btnState = 0;
    xil_printf("Switches, Buttons and LEDs task\r\n");
    
    while (1) {
        
        swState = XGpio_DiscreteRead(&Gpio, SW_CHANNEL);
        btnState = XGpio_DiscreteRead(&Gpio_btns, BTN_CHANNEL);
        if(btnState > 0){
            ledValue = swState;
        }else{

            ledValue = 0x0000;
        }
        XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, ledValue);
        vTaskDelay(x100ms);
    }

}

