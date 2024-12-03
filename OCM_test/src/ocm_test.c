/*
*
* Simple OCM test that periodically reads an OCM memory address and change gpio state according to the value on that address
* Core 0 is responsible for changing the value -> not reached yet
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
int LED1 = 0x1; // LED1 mask
/*-----------------------------------------------------------*/


/*---------------------- OCM Parameters---------------------*/
#define TARGET_ADDR 0xFFFC0000 // Dirección prueba comm OCM
#define OCM_CFG_ADDR 0x0000001F // OCM_CFG direction
#define ADDR_OFFSET (*(volatile uint32_t *)(0x45))          // Offset for OCM adddr(offset de 69 -> 0xFFFC0045)
/*-----------------------------------------------------------*/


/*---------------------- freeRTOS task parameters------------*/
#define DELAY_100_ms        100UL
#define DELAY_500_ms        500UL
static void checkOCM_Task( void *pvParameters );
static void checkBTNS_Task( void *pvParameters );
static TaskHandle_t xThread1;
static TaskHandle_t xThread2;
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
    volatile uint32_t *OCM_CFG = (uint32_t *)OCM_CFG_ADDR;

    
    *OCM_CFG = 0x0000001F; // OCM relocation enabled
    Xil_DCacheFlush();
    //Xil_Out32(0xF8000910, 0x0000001F); // OCM relocation -> high address config
    printf("Direccion del puntero de OCM config %08x\r\n", &OCM_CFG);
    printf("Valor del puntero en OCM_CFG: %08x\r\n", (*OCM_CFG));
    printf("Valor con Xil_in32 del OCM_CFG: %08x\r\n", Xil_In32(OCM_CFG_ADDR));
    xil_printf("MAIN of GPIO test. Starting GPIO config...\r\n");

    Xil_SetTlbAttributes(0xFFFC0000,0x14de2); // descached OCM space

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

    /* Task Creation */
	xTaskCreate( 	checkOCM_Task, 					// The function that implements the task. 
					( const char * ) "Thread1", 	// Text name for the task, provided to assist debugging only.
					configMINIMAL_STACK_SIZE, 	// The stack allocated to the task.
					NULL, 						// The task parameter is not used, so set to NULL. 
					tskIDLE_PRIORITY,			// The task runs at the idle priority (priority zero). 
					&xThread1 );

        /* Task Creation */
/* 	xTaskCreate( 	checkBTNS_Task, 					// The function that implements the task. 
					( const char * ) "Thread1", 		// Text name for the task, provided to assist debugging only. 
					configMINIMAL_STACK_SIZE, 	// The stack allocated to the task. 
					NULL, 						// The task parameter is not used, so set to NULL. 
					tskIDLE_PRIORITY,		// The task runs at the idle priority + 1 (higher priority than idle). 
					&xThread1 ); */


	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );

}

static void checkOCM_Task( void *pvParameters ){

    uint32_t addrValue = 0;
    uint32_t ledValue = 0x0F;
    //addrValue = 0x0000000F;
    while(1){

        addrValue = Xil_In32(TARGET_ADDR);
        //addrValue = TARGET_ADDR;
        xil_printf("Addr Value: %08x\r\n", addrValue);
        // 0xE92D00F0 
        if(addrValue > 0){ // any of 4 low bits enabled -> led enable. Old condition: (addrValue & (0x0000000F)) != 0
            //ledValue = 0x0F;
            //XGpio_DiscreteWrite(&Gpio, CH1, ledValue);
            //Xil_Out32((TARGET_ADDR + ADDR_OFFSET) , 0x000000FF);
            
            xil_printf("Holaaaaa\r\n");
        }
        XGpio_DiscreteWrite(&Gpio, CH1, ledValue); 
        vTaskDelay(x500ms);
    }
} 


static void checkBTNS_Task( void *pvParameters ){

    uint32_t ledValue = 0; // turn off
    uint32_t btnState = 0;
    uint32_t swState = 0;
    uint32_t auxRead = 0; // aux variable to check writing of core 1 on OCM address
    uint32_t auxValue = 0xFFFF0000; // specified value for turn led off without button

    while(1){
        
        //auxRead = Xil_In32(TARGET_ADDR);
        swState = XGpio_DiscreteRead(&Gpio, CH2);
        btnState = XGpio_DiscreteRead(&Gpio_btns, CH1);

        if(btnState > 0){
            ledValue = swState;
        }else{
            ledValue = 0x0000;
        }

        XGpio_DiscreteWrite(&Gpio, CH1, ledValue);
        vTaskDelay(x100ms);

/*         if((auxRead == auxValue) ){
            //XGpio_DiscreteWrite(&Gpio_btns, CH1, ledValue);
            Xil_Out32((TARGET_ADDR + ADDR_OFFSET) , 0x00000000);
        } */
        
    }
}
