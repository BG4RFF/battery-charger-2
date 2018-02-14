#include <driverlib.h>
#include <LCD.h>


#define TIMER_A_PERIOD 35
int DUTY_CYCLE = 35;

uint16_t i=0;
float Adc_Value=0;
uint16_t Adc_Value1=0;
void Timer_A_outputPWM_implementation(uint16_t baseAddress,
                       Timer_A_outputPWMParam *param)
{
    HWREG16(baseAddress + OFS_TAxCTL) &=
        ~(TIMER_A_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMER_A_UPDOWN_MODE + TIMER_A_DO_CLEAR +
          TIMER_A_TAIE_INTERRUPT_ENABLE +
          ID__8
          );
    HWREG16(baseAddress + OFS_TAxEX0) &= ~TAIDEX_7;

    HWREG16(baseAddress + OFS_TAxEX0) |= param->clockSourceDivider & 0x7;
    HWREG16(baseAddress + OFS_TAxCTL) |= (param->clockSource +
                                          TIMER_A_UP_MODE +
                                          TIMER_A_DO_CLEAR +
                                          ((param->clockSourceDivider >>
                                            3) << 6));

    HWREG16(baseAddress + OFS_TAxCCR0) = param->timerPeriod;

    HWREG16(baseAddress + OFS_TAxCCTL0) &=
        ~(TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMER_A_OUTPUTMODE_RESET_SET);

    HWREG16(baseAddress + TIMER_A_CAPTURECOMPARE_REGISTER_2) |= TIMER_A_OUTPUTMODE_RESET_SET;

    HWREG16(baseAddress + TIMER_A_CAPTURECOMPARE_REGISTER_2 + OFS_TAxR) =param->dutyCycle;
  
    HWREG16(baseAddress + TIMER_A_CAPTURECOMPARE_REGISTER_1) |=TIMER_A_OUTPUTMODE_SET_RESET;

    HWREG16(baseAddress +TIMER_A_CAPTURECOMPARE_REGISTER_1+ OFS_TAxR) = param->dutyCycle;
}




int main(void) {
  
  
   // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);
    
    PMM_unlockLPM5();
  ///////////////////PWM configuration///////////////////////////
     GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
        );

     //P3.3 as PWM output
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P3,
        GPIO_PIN3,
        GPIO_SECONDARY_MODULE_FUNCTION
        );
CS_initClockSignal(CS_SMCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_1);
CS_setDCOFreq(CS_DCORSEL_0,CS_DCOFSEL_6);
    //Generate PWM - Timer runs in Up-Down mode
    Timer_A_outputPWMParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = TIMER_A_PERIOD;
  
   
    //////////////////////PWM conf. end///////////////////////////////

    
//**************ADC12_B*************************//
 /* Select Port 1
    * Set Pin 8.5 to output (or Input I/O x) Ternary Module Function, (A6, C7).
    */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
    	GPIO_PORT_P8,
    	GPIO_PIN5,
    	GPIO_TERNARY_MODULE_FUNCTION
    );
 
     //Initialize the ADC12B Module
    /*
    * Base address of ADC12B Module
    * Use internal ADC12B bit as sample/hold signal to start conversion
    * USE MODOSC 5MHZ Digital Oscillator as clock source
    * Use default clock divider/pre-divider of 1
    * Not use internal channel
    */
   	ADC12_B_initParam initParam = {0};
    initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
    initParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_ADC12OSC;
    initParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
    initParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
    initParam.internalChannelMap = ADC12_B_NOINTCH;
	ADC12_B_init(ADC12_B_BASE, &initParam);

    //Enable the ADC12B module
    ADC12_B_enable(ADC12_B_BASE);
    
     /*
    * Base address of ADC12B Module
    * For memory buffers 0-7 sample/hold for 64 clock cycles
    * For memory buffers 8-15 sample/hold for 4 clock cycles (default)
    * Disable Multiple Sampling
    */
    ADC12_B_setupSamplingTimer(ADC12_B_BASE,
      ADC12_B_CYCLEHOLD_16_CYCLES,
      ADC12_B_CYCLEHOLD_4_CYCLES,
      ADC12_B_MULTIPLESAMPLESDISABLE);

   
 //Configure Memory Buffer for A5
    /*
    * Base address of the ADC12B Module
    * Configure memory buffer 1
    * Map input A6 to memory buffer 1
    * Vref+ = AVcc
    * Vref- = AVss
    * Memory buffer 0 is not the end of a sequence
    */
    ADC12_B_configureMemoryParam configureMemoryParam1 = {0};
    configureMemoryParam1.memoryBufferControlIndex = ADC12_B_MEMORY_0;
    configureMemoryParam1.inputSourceSelect = ADC12_B_INPUT_A6;
    configureMemoryParam1.refVoltageSourceSelect = ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
    configureMemoryParam1.endOfSequence = ADC12_B_NOTENDOFSEQUENCE;
    configureMemoryParam1.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
    configureMemoryParam1.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
   ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam1);

    
 ///////////////ADC////////////////////////    

    LCDInit(LS_NONE);
    while(1)
    {
        
       

        
         
          LCDClear();
          
            LCDWriteIntXY(0,1,Adc_Value,4);
            
          for(i=0;i<10000;i++);
          
        
    

  ADC12_B_startConversion(ADC12_B_BASE,
            ADC12_B_MEMORY_0,
            ADC12_B_SINGLECHANNEL);
        // Delay
        for(i=10000; i>0; i--);
       
        
        
   Adc_Value=ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
    Adc_Value=Adc_Value*(TIMER_A_PERIOD/4095.0);
  DUTY_CYCLE=(int)Adc_Value;
 param.dutyCycle = DUTY_CYCLE;
    Timer_A_outputPWM_implementation(TIMER_A1_BASE, &param);   
     

    } //End of while loop 
}
