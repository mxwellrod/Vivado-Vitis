/*
*
* Creation of OCM buffer for testing purposes. If any button is pressed then buffer overwrites buffer value with -value
* Use within core0 running linux
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
#include "xil_cache.h"

/*---------------------- GPIO Parameters---------------------*/
#define CH1 1
#define CH2 2
#define XGPIO_AXI_LED_BASEADDR XPAR_XGPIO_0_BASEADDR
#define XGPIO_AXI_BTN_BASEADDR XPAR_XGPIO_1_BASEADDR
XGpio Gpio; // drivers instances defined globally for task
XGpio Gpio_btns; // drivers instances defined globally for task
/*-----------------------------------------------------------*/


/*---------------------- OCM Parameters---------------------*/
#define OCM_BASE_ADDR_LOW 0x00000000  // freeRTOS base ADDR for 3 low address OCM blocks(OCM0 - OCM2)
#define OCM_BASE_ADDR_HIGH 0xFFFF0000 // freeRTOS base ADDR for last OCM block located on a high address(OCM3)
#define MAX 65536                     // mem vector max size for all OCM -> (256 KB / 4 bytes per float)
#define MAX_HIGH 16384               // mem high max size -> (64 KB / 4 bytes per float)
#define MEM_SIZE (MAX * sizeof(float))  // Vector size in bytes
volatile float *mem_low = (float *)OCM_BASE_ADDR_LOW; // mem_low points to low ocm base addr
volatile float *mem_high = (float *)OCM_BASE_ADDR_HIGH; //mem high points to high ocm base addr
/*-----------------------------------------------------------*/


/*---------------------- freeRTOS task parameters------------*/
#define DELAY_100_ms        100UL
#define DELAY_500_ms        500UL
static void checkBTNS_Task( void *pvParameters );
static void checkSWS_Task( void *pvParameters );
static TaskHandle_t xThread1;
static TaskHandle_t xThread2;
const TickType_t x100ms = pdMS_TO_TICKS( DELAY_100_ms);
const TickType_t x500ms = pdMS_TO_TICKS( DELAY_500_ms);
/*-----------------------------------------------------------*/


int main( void ){


    int status = 0;
    XGpio_Config *CfgPtr;

    xil_printf("Entered main. Starting GPIO config...\r\n");

    //Xil_SetTlbAttributes(0xFFFC0000,0x14de2); // descached OCM space

    // GPIO config leds
    CfgPtr = XGpio_LookupConfig(XGPIO_AXI_LED_BASEADDR);
    if(CfgPtr == NULL){
        xil_printf("Error looking up the leds gpio config\r\n");
        return XST_FAILURE;
    }
    status = XGpio_CfgInitialize(&Gpio, CfgPtr, CfgPtr -> BaseAddress);
    if(status != XST_SUCCESS){
        xil_printf("Config Initialization of leds failed\r\n");
        return XST_FAILURE;
    }

    status = XGpio_Initialize(&Gpio, CfgPtr -> BaseAddress);
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
    status = XGpio_CfgInitialize(&Gpio_btns, CfgPtr, CfgPtr -> BaseAddress);
    if(status != XST_SUCCESS){
        xil_printf("Config initialization of Buttons failed\r\n");
        return XST_FAILURE;
    }

    status = XGpio_Initialize(&Gpio_btns, CfgPtr -> BaseAddress);
    if(status != XST_SUCCESS){
        xil_printf("GPIO Initialization of Buttons failed\r\n");
        return XST_FAILURE;
    } 

    // GPIO direction settings leds + buttons
    XGpio_SetDataDirection(&Gpio, CH1, 0x00);  // output
    XGpio_SetDataDirection(&Gpio, CH2, 0xFF); // input
    XGpio_SetDataDirection(&Gpio_btns, CH1, 0xFF); // input

    xil_printf("GPIO config finished. Task creation...\r\n");


    /* Task Creation */
	xTaskCreate( 	checkBTNS_Task, 					// The function that implements the task. 
					( const char * ) "Thread1", 		// Text name for the task, provided to assist debugging only. 
					configMINIMAL_STACK_SIZE, 	// The stack allocated to the task. 
					NULL, 						// The task parameter is not used, so set to NULL. 
					tskIDLE_PRIORITY,		// The task runs at the idle priority
					&xThread1 );

        /* Task Creation */
	xTaskCreate( 	checkSWS_Task, 					// The function that implements the task. 
					( const char * ) "Thread2", 		// Text name for the task, provided to assist debugging only. 
					configMINIMAL_STACK_SIZE, 	// The stack allocated to the task. 
					NULL, 						// The task parameter is not used, so set to NULL. 
					tskIDLE_PRIORITY,		// The task runs at the idle priority
					&xThread2 );


	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );

}

static void checkBTNS_Task( void *pvParameters ){
    uint32_t btnState = 0;

    while(1){
        btnState = XGpio_DiscreteRead(&Gpio_btns, CH1); // read buttons state

        if(btnState > 0){ // any button pressed -> overwrite mem vector with -i value
            for(int i = 0; i < MAX - MAX_HIGH; i++){ // mem low writting ends on element #49152
                mem_low[i] = -((float)i);
            }

            for(int j = MAX - MAX_HIGH; j < MAX; j++){
                mem_high[j - (MAX - MAX_HIGH)] = -((float)j);
            }

            xil_printf("Memory updated on core1.\n");
            XGpio_DiscreteWrite(&Gpio, CH1, 0x0F); // read complete -> turn on leds for a brief moment

        }

        vTaskDelay(x100ms);
        XGpio_DiscreteWrite(&Gpio, CH1, 0x00); // read complete -> turn on leds for a brief moment
    }

}

static void checkSWS_Task( void *pvParameters ){
    uint32_t swState = 0;

    while(1){
        swState = XGpio_DiscreteRead(&Gpio, CH2);

        if(swState > 0){
            xil_printf("switch enabled -> Print mem values");
        
            // print mem_low -> works but too long
            /* for (int i = 0; i < MAX - MAX_HIGH; i++) {
                printf("mem_low[%d] = %.2f\n", i, mem_low[i]);  // %.2f -> two decimals
            } */

            // print mem_high (OCM3)
            for (int j = 0; j < MAX_HIGH; j++) {
                printf("mem_high[%d] = %.2f\n", j, mem_high[j]);  // %.2f -> two decimals
            }
            
        }

        vTaskDelay(x500ms);

    }
    
}