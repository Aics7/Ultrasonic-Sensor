#include "mkl25z4.h"
#include "ultrasonic.h"

#define MASK(x)	(1UL << x)
#define TRIG		(2)		//PTB2
#define ECHO		(3)		//PTB3

volatile int counter = 1;
unsigned int interval=7;

void init_pin_IC(){
	SIM->SCGC5 |=SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[ECHO] &= ~PORT_PCR_MUX_MASK; //Clear mux
	PORTB->PCR[ECHO] |= PORT_PCR_MUX(3); //setup to be output of TPM1_CH0
}

void init_Timer_IC()
{
	//Clock gate
	SIM->SCGC6 |=SIM_SCGC6_TPM2_MASK;	//*******TPM2 channel 0
	//Select clock source in SIM_SOPT
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);	//1- MCGPLLCLK/2, or MCGFLLCLK 01 (ext?), 2-ext OSCERCLK, 3-internal MCGIRCLK
	//Configure registers
	TPM2->MOD= 0x3333; //80ms

	//working with TPM2_C0SC
	//input capture
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_MSB(0) |TPM_CnSC_MSA(0) |TPM_CnSC_ELSB(1)| TPM_CnSC_ELSA(1);
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_CHF_MASK;  //clear spurious interrupts
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_CHIE(1);
	TPM2->SC |=  TPM_SC_TOF_MASK | TPM_SC_PS(1) | TPM_SC_TOIE_MASK  ;
	TPM2->SC |= TPM_SC_CMOD(1); //enable internal clock to run

	NVIC_ClearPendingIRQ(TPM2_IRQn);
	NVIC_SetPriority(TPM2_IRQn, 3);
	NVIC_EnableIRQ(TPM2_IRQn);
}

void TPM2_IRQHandler()
{
	static int ctr=0;
	static unsigned int previous=0;
	unsigned int current=0;
	if (TPM2->STATUS & TPM_STATUS_CH1F_MASK)
	{// check if input occurred
		current=TPM2->CONTROLS[1].CnV;
		current |= (ctr <<16); // add the no. of overflows.
		//Each ctr tick is 2^16,
		//without above, current value could also be more than prev.
		interval = current-previous;
		previous=current;
		TPM2->CONTROLS[1].CnSC |=TPM_CnSC_CHF_MASK; //clear input capture flag
	}
	if (TPM2->SC & TPM_SC_TOF_MASK)
	{
		ctr++; //a timer overflow occurred.
		TPM2->SC |= TPM_SC_TOF_MASK ; //clear the interrupt on timer overflow
	}
}

void initialize_pit()
{
  SIM->SCGC5 |=SIM_SCGC5_PORTB_MASK;
  PORTB->PCR[TRIG] &= ~PORT_PCR_MUX_MASK;  //Clear mux
  PORTB->PCR[TRIG] |= PORT_PCR_MUX(1);  //setup to be GPIO
  PTB->PDDR |= MASK(TRIG) ;
	PTB->PCOR =MASK(TRIG);  //set TRIG low
}

void initialize_timer()
{
  //Clock gate
  SIM->SCGC6 |=SIM_SCGC6_PIT_MASK;
  //enable PIT timer
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;
  //select channel 0 and load value of 0x00422444, approximately 400ms
  PIT->CHANNEL[0].LDVAL =  0xD2; //80ms
  //enable PIT timer, interrupt and chain mode 
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK | PIT_TCTRL_CHN_MASK;
  //clear flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

  //configure PIT IRQ
  NVIC_ClearPendingIRQ(PIT_IRQn);
  NVIC_SetPriority(PIT_IRQn, 3);
  NVIC_EnableIRQ(PIT_IRQn);
  //__enable_irq();
}

void PIT_IRQHandler(){
  PTB->PTOR |= MASK(TRIG);
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK; //clear the interrupt
  
  if (counter == 1){
    PIT->CHANNEL[0].LDVAL = 0xCCCC0;
  }
  if(counter == 2){
    PIT->CHANNEL[0].LDVAL = 0xD2;
    counter = 0;
  }
  counter ++;
}

int get_distance()
{
	return interval/(1715*2);
}
