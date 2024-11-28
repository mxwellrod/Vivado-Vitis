/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_io.h" 
#include "xil_mmu.h"
#include "xuartps.h"

#define LED_CHANNEL 1
#define SW_CHANNEL 2
#define XGPIO_AXI_BASEADDR XPAR_XGPIO_0_BASEADDR
#define DELAY_100_ms        100UL
#define DELAY_500_ms        500UL

/* The Tx and Rx tasks as described at the top of this file. */
static void checkOCM_Task( void *pvParameters );
static TaskHandle_t xThread1;
const TickType_t x100ms = pdMS_TO_TICKS( DELAY_100_ms);
const TickType_t x500ms = pdMS_TO_TICKS( DELAY_500_ms);

XGpio Gpio;
XGpio Gpio_btns;

int LED1 = 0x1;

int main( void ){

    int status = 0;
    XGpio_Config *CfgPtr;
    XGpio_Config *CfgPtrBtns;

    xil_printf( "freeRTOS main. OCM demo\r\n" );

    // GPIO config leds
    CfgPtr = XGpio_LookupConfig(XGPIO_AXI_BASEADDR);
    status = XGpio_CfgInitialize(&Gpio, CfgPtr, XGPIO_AXI_BASEADDR);

    if(status != XST_SUCCESS){
        xil_printf("LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    status = XGpio_Initialize(&Gpio, XGPIO_AXI_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("GPIO Initialization failed\r\n");
        return XST_FAILURE;
    }


    // GPIO config butttons
    CfgPtrBtns = XGpio_LookupConfig(XPAR_XGPIO_1_BASEADDR);
    status = XGpio_CfgInitialize(&Gpio_btns, CfgPtrBtns, XPAR_XGPIO_1_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("LookupConfig Buttons failed\r\n");
        return XST_FAILURE;
    }

    status = XGpio_Initialize(&Gpio_btns, XGPIO_AXI_BASEADDR);
    if(status != XST_SUCCESS){
        xil_printf("GPIO Initialization Buttons failed\r\n");
        return XST_FAILURE;
    }
    


    // GPIO direction settings leds + buttons
    XGpio_SetDataDirection(&Gpio, LED_CHANNEL, 0x00);  // output
    XGpio_SetDataDirection(&Gpio, SW_CHANNEL, 0xFF); // input
    XGpio_SetDataDirection(&Gpio_btns, 1, 0xFF); // input

	/* Create the two tasks.  The Tx task is given a lower priority than the
	Rx task, so the Rx task will leave the Blocked state and pre-empt the Tx
	task as soon as the Tx task places an item in the queue. */
	xTaskCreate( 	checkOCM_Task, 					/* The function that implements the task. */
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
static void checkOCM_Task( void *pvParameters )
{
    uint32_t ledValue = 0;
    uint32_t swState = 0;
    uint32_t btnState = 0;
    xil_printf("Swtiches, Buttons and LEDs task\r\n");
    
    while (1) {
        
        swState = XGpio_DiscreteRead(&Gpio, SW_CHANNEL);
        printf("Estado de los switches: %08x\r\n", swState);
        btnState = XGpio_DiscreteRead(&Gpio_btns, 1);
        printf("Estado de los botones: %08x\r\n", btnState);
        
        XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, LED1|=0b0001); // se enciende
        vTaskDelay(x500ms); // Delay to avoid overload (monitorea cada 100 ms)
        XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, LED1&=0b0000); // se apaga 
        vTaskDelay(x500ms); // Delay to avoid overload (monitorea cada 100 ms)
    }
}

