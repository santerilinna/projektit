


//include stdio
#include <stdio.h>

//xdc includes
#include <xdc/std.h>
#include <xdc/runtime/System.h>

// BIOS Header files
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/i2c/I2CCC26XX.h>

//Board header files
#include "Board.h"
#include "wireless/comm_lib.h"
#include "sensors/mpu9250.h"
#include "buzzer.h"

//taskstack
#define STACKSIZE 2048
Char sensorTaskStack[STACKSIZE];
Char uartTaskStack[STACKSIZE];

// statemachine
enum state { WAITING=1, YMOVE, JUMPED, XMOVE, BUTTON1, BEEPED };
enum state programState = WAITING;

// handles and states
static PIN_Handle powerButtonHandle;
static PIN_State powerButtonState;
static PIN_Handle buttonHandle;
static PIN_State buttonState;
static PIN_Handle led0Handle;
static PIN_Handle led1Handle;
static PIN_State ledState;
static PIN_Handle hBuzzer;
static PIN_State sBuzzer;
static PIN_Handle hMpuPin;
static PIN_State  MpuPinState;

//pin configs
PIN_Config powerButtonConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE //always end with PIN_TERMINATE
};
PIN_Config powerButtonWakeConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
   PIN_TERMINATE
};

PIN_Config buttonConfig[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};

PIN_Config led0Config[] = {
   Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
   PIN_TERMINATE
};
PIN_Config led1Config[] = {
   Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
   PIN_TERMINATE
};

PIN_Config cBuzzer[] = {
  Board_BUZZER | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
  PIN_TERMINATE
};


// MPU power pin
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

// MPU uses its own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

Void powerFxn(PIN_Handle handle, PIN_Id pinId) {
    // puts machine to a sleep state
    if (programState == WAITING) {
          Task_sleep(100000 / Clock_tickPeriod);
          PIN_close(powerButtonHandle);
          PINCC26XX_setWakeup(powerButtonWakeConfig);
          Power_shutdown(NULL,0);
    }

}

void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
    //just turns led on/off
    uint_t pinValue = PIN_getOutputValue( Board_LED0 );
    pinValue = !pinValue;
    PIN_setOutputValue( led0Handle, Board_LED0, pinValue );
    uint_t pinValue2 = PIN_getOutputValue( Board_LED1 );
    pinValue2 = !pinValue2;
    PIN_setOutputValue( led1Handle, Board_LED1, pinValue2 );
    if (programState == WAITING) {
        programState = BUTTON1;
    }
}

void playSound(value) {
    // plays chosen sound effect
    if (value == 1) {   //sound effect 1
        buzzerOpen(hBuzzer);
        buzzerSetFrequency(988);
        Task_sleep(100000 / Clock_tickPeriod);
        buzzerSetFrequency(1319);
        Task_sleep(400000 / Clock_tickPeriod);
        buzzerClose();
    }
    if (value == 2) {   //sound effect 2
        buzzerOpen(hBuzzer);
        buzzerSetFrequency(1319);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerSetFrequency(1568);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerSetFrequency(2637);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerSetFrequency(2093);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerSetFrequency(2349);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerSetFrequency(3136);
        Task_sleep(130000 / Clock_tickPeriod);
        buzzerClose();
    }
    if (value == 3) {   // sound effect 3
        buzzerOpen(hBuzzer);
        buzzerSetFrequency(800);
        Task_sleep(200000 / Clock_tickPeriod);
        buzzerSetFrequency(1000);
        Task_sleep(200000 / Clock_tickPeriod);
        buzzerSetFrequency(1200);
        Task_sleep(200000 / Clock_tickPeriod);
        buzzerSetFrequency(800);
        Task_sleep(200000 / Clock_tickPeriod);
        buzzerClose();
    }
    if (value == 4) {   // warning sound
        buzzerOpen(hBuzzer);
        buzzerSetFrequency(990);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerSetFrequency(930);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerSetFrequency(990);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerSetFrequency(930);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerSetFrequency(990);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerSetFrequency(930);
        Task_sleep(140000 / Clock_tickPeriod);
        buzzerClose();
    }
     if (value == 5) {  //startup sound
         buzzerOpen(hBuzzer);
         buzzerSetFrequency(1000);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerSetFrequency(1200);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerSetFrequency(1400);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerSetFrequency(1300);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerSetFrequency(1200);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerSetFrequency(1400);
         Task_sleep(300000 / Clock_tickPeriod);
         buzzerClose();
    }
}

//global variable for movement action
int movement_check;

void motionDetect(float ax_dataset[], float ay_dataset[], float az_dataset[]) {
    int i, j;
    movement_check = 0;
    for(i=0;i<80;i++) {

        //detect x-axis back&forth motion
        if(ax_dataset[i]>0.5) {
            for(j=i;j<80;j++) {
                if(ax_dataset[j]<-0.5) {
                    movement_check = 3;
                }
            }
        }
        if(ax_dataset[i]<-0.5) {
            for(j=i;j<80;j++) {
                if(ax_dataset[j]>0.5) {
                    movement_check = 3;
                }
            }
        }

        //detect y-axis back&forth motion
        if(ay_dataset[i]>0.5) {
            for(j=i;j<80;j++) {
                if(ay_dataset[j]<-0.5) {
                    movement_check = 2;
                }
            }
        }
        if(ay_dataset[i]<-0.5) {
            for(j=i;j<80;j++) {
                if(ay_dataset[j]>0.5) {
                    movement_check = 2;
                }
            }
        }

        //detect jumping motion (value of z-accel when stationary is -1)
        if(az_dataset[i]>-0.7) {
            movement_check = 1;
        }
    }
}

static void recvFxn(UART_Handle uart, void *rxBuf, size_t len) {
    // UART receive handle function
    if(strstr(rxBuf,"2251")){
        if(programState == WAITING) {
        programState = BEEPED;
        }
    }
    UART_read(uart, rxBuf, 80); //continue UART_read
}

// Task Functions
void uartTaskFxn(UArg arg0, UArg arg1) {
    Board_initUART();
    UART_Handle uart;
    UART_Params uartParams;
    UART_Params_init(&uartParams); //init
    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readMode=UART_MODE_CALLBACK;
    uartParams.readCallback = &recvFxn;
    uartParams.baudRate = 9600; // speed 9600baud
    uartParams.dataLength = UART_LEN_8; // 8
    uartParams.parityType = UART_PAR_NONE; // n
    uartParams.stopBits = UART_STOP_ONE; // 1
    uart = UART_open(Board_UART0, &uartParams); //open connection to Board_UART0
    if (uart == NULL) {
        System_abort("Error opening the UART");
    }
    char uart_msg[64];      //sending buffer
    uint8_t uartBuffer[80]; //receiving buffer
    UART_read(uart, uartBuffer, 80);
    while (1) {
        if (programState == BEEPED) {   //plays warning sound when receiving uart messages meant for our sensortag
            playSound(4);
            programState = WAITING;
            UART_read(uart, uartBuffer, 80);
        }
        if (programState == JUMPED) {   //sends message to eat when jump motion is detected
            sprintf(uart_msg,"id:2251,EAT:1\0");
            UART_write(uart,uart_msg, strlen(uart_msg) + 1);
            programState = WAITING;
            }
        if (programState == YMOVE) { //sends message to exercise when y-axis movement is detected
            sprintf(uart_msg,"id:2251,EXERCISE:1\0");
            UART_write(uart,uart_msg, strlen(uart_msg) + 1);
            programState = WAITING;
            }
        if (programState == XMOVE) {    //sends message to exercise when x-axis movement is detected
            sprintf(uart_msg,"id:2251,PET:1\0");
            UART_write(uart,uart_msg, strlen(uart_msg) + 1);
            programState = WAITING;
        }
        if (programState == BUTTON1) {  //sends message when sidebutton is pressed, kind of unused
            sprintf(uart_msg,"id:2251,MSG2:napin käyttötarkoitus = ?,MSG1:sivunappia painettu\0");
            UART_write(uart,uart_msg, strlen(uart_msg) + 1);
            programState = WAITING;
        }
        Task_sleep(100000 / Clock_tickPeriod); //10 times per second
    }
}

void sensorTaskFxn(UArg arg0, UArg arg1) {
    Board_initI2C();
    float ax, ay, az, gx, gy, gz;
    I2C_Handle i2cMPU; // Own i2c-interface for MPU9250 sensor
    I2C_Params i2cMPUParams;
    I2C_Params_init(&i2cMPUParams);
    i2cMPUParams.bitRate = I2C_400kHz;
    i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON); //MPU power on
    Task_sleep(100000 / Clock_tickPeriod); //wait 100ms for MPU power on
    System_printf("MPU9250: Power ON\n");
    System_flush();
    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams); // open mpu i2c
    if (i2cMPU == NULL) {
        System_abort("Error Initializing I2CMPU\n");
    }
    System_printf("MPU9250: Setup and calibration...\n");
    System_flush();
    playSound(5);
    mpu9250_setup(&i2cMPU);
    System_printf("MPU9250: Setup and calibration OK\n");
    System_flush();
    while(1) {
        //arrays for motionDetect function
        float ax_dataset[80];
        float ay_dataset[80];
        float az_dataset[80];
        int i=0;

        while(i<80) {
            mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
            ax_dataset[i] = ax;
            ay_dataset[i] = ay;
            az_dataset[i] = az;
            i++;
            Task_sleep(25000 / Clock_tickPeriod);
        }
        motionDetect(ax_dataset, ay_dataset, az_dataset);

        if (movement_check == 1) {      // jump/z-axis movement
            playSound(1);               //play sound effect 1
            if (programState == WAITING) { //state change
                programState = JUMPED;
            }
            Task_sleep(250000 / Clock_tickPeriod);
        }
        if (movement_check == 2) {      // y-axis movement
             playSound(2);              //play sound effect 2
             if (programState == WAITING) { //state change
                 programState = YMOVE;
             }
             Task_sleep(250000 / Clock_tickPeriod);
            }
        if (movement_check == 3) {      // x-axis movement
            playSound(3);               //play sound effect 3
            if (programState == WAITING) { //state change
                programState = XMOVE;
            }
            Task_sleep(250000 / Clock_tickPeriod);
            }
        Task_sleep(10000 / Clock_tickPeriod); //small delay only needed
    }
}

int main(void) {
    // Task variables
    Task_Handle sensorTaskHandle;
    Task_Params sensorTaskParams;
    Task_Handle uartTaskHandle;
    Task_Params uartTaskParams;

    // Initialize board
    Board_initGeneral();
    Init6LoWPAN(); //unused

    //open pins
    hBuzzer = PIN_open(&sBuzzer, cBuzzer);  //buzzer
    if (hBuzzer == NULL) {
        System_abort("Pin open failed!");
    }
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig); //MPU pin
    if (hMpuPin == NULL) {
        System_abort("Pin open failed!");
    }
    buttonHandle = PIN_open(&buttonState, buttonConfig); //sidebutton
    if(!buttonHandle) {
        System_abort("Error initializing button pins\n");
    }
    led0Handle = PIN_open(&ledState, led0Config); //led1
    if(!led0Handle) {
        System_abort("Error initializing LED pins\n");
    }
    led1Handle = PIN_open(&ledState, led1Config); //led2
    if(!led1Handle) {
        System_abort("Error initializing LED pins\n");
    }
    if (PIN_registerIntCb(buttonHandle, &buttonFxn) != 0) {
        System_abort("Error registering button callback function");
    }
    powerButtonHandle = PIN_open(&powerButtonState, powerButtonConfig); //powerbutton
    if(!powerButtonHandle) {
        System_abort("Error initializing power button\n");
    }
    if (PIN_registerIntCb(powerButtonHandle, &powerFxn) != 0) {
        System_abort("Error registering power button callback");
    }

    //Task
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTaskParams.priority=2;
    sensorTaskHandle = Task_create(sensorTaskFxn, &sensorTaskParams, NULL);
    if (sensorTaskHandle == NULL) {
        System_abort("Task create failed!");
    }
    Task_Params_init(&uartTaskParams);
    uartTaskParams.stackSize = STACKSIZE;
    uartTaskParams.stack = &uartTaskStack;
    uartTaskParams.priority=2;
    uartTaskHandle = Task_create(uartTaskFxn, &uartTaskParams, NULL);
    if (uartTaskHandle == NULL) {
        System_abort("Task create failed!");
    }

    //hello world
    System_printf("Hello world!\n");
    System_flush();

    //start bios
    BIOS_start();
    return (0);
}
