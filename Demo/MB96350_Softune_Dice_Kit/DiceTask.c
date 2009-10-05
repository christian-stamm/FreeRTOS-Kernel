/*
    FreeRTOS V6.0.0 - Copyright (C) 2009 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it    under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation and modified by the FreeRTOS exception.
    **NOTE** The exception to the GPL is included to allow you to distribute a
    combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    Alternative commercial license and support terms are also available upon
    request.  See the licensing section of http://www.FreeRTOS.org for full
    license details.

    FreeRTOS is distributed in the hope that it will be useful,    but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with FreeRTOS; if not, write to the Free Software Foundation, Inc., 59
    Temple Place, Suite 330, Boston, MA  02111-1307  USA.


    ***************************************************************************
    *                                                                         *
    * The FreeRTOS eBook and reference manual are available to purchase for a *
    * small fee. Help yourself get started quickly while also helping the     *
    * FreeRTOS project! See http://www.FreeRTOS.org/Documentation for details *
    *                                                                         *
    ***************************************************************************

    1 tab == 4 spaces!

    Please ensure to read the configuration and relevant port sections of the
    online documentation.

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


/* 
 * Defines the 'dice' tasks as described at the top of main.c
 */


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Delays used within the dice functionality.  All delays are defined in milliseconds. */
#define diceDELAY_BETWEEN_RANDOM_NUMBERS_ms		( 20 / portTICK_RATE_MS )
#define diceSHAKE_TIME							( ( 2000 / portTICK_RATE_MS ) / diceDELAY_BETWEEN_RANDOM_NUMBERS_ms )
#define diceSHORT_PAUSE_BEFORE_SHAKE			( 250 / portTICK_RATE_MS )
#define diceDELAY_WHILE_DISPLAYING_RESULT		( 5000 / portTICK_RATE_MS )

/* Macro to access the display ports. */
#define dice7SEG_Value( x )		( *( pucDisplayOutput[ x ] ) )

/* Checks the semaphore use to communicate button push events.  A block time
can be specified - this is the time to wait for a button push to occur should
one have not already occurred. */
#define prvButtonHit( ucIndex, xTicksToWait ) xSemaphoreTake( xSemaphores[ ucIndex ], xTicksToWait )

/* Defines the outputs required for each digit on the display. */
static const char cDisplaySegments[ 2 ][ 11 ] =
{
	{ 0x48, 0xeb, 0x8c, 0x89, 0x2b, 0x19, 0x18, 0xcb, 0x08, 0x09, 0xf7 }, /* Left display. */
	{ 0xa0, 0xf3, 0xc4, 0xc1, 0x93, 0x89, 0x88, 0xe3, 0x80, 0x81, 0x7f }  /* Right display. */
};

/* The semaphores used to communicate button push events between the button
input interrupt handlers and the dice tasks.  Two dice tasks are created so two
semaphores are required. */
static xSemaphoreHandle xSemaphores[ 2 ] = { 0 };

/* Defines the ports used to write to the display.  This variable is defined in
partest.c, which contains the LED set/clear/toggle functions. */
extern volatile unsigned char *pucDisplayOutput[ 2 ];

/*-----------------------------------------------------------*/

/* 
 * Defines the 'dice' tasks as described at the top of main.c
 */
void vDiceTask( void *pvParameters )
{
unsigned char ucDiceValue, ucIndex;
unsigned long ulDiceRunTime;
extern void vSuspendFlashTasks( unsigned char ucIndex, short sSuspendTasks );



	/* Two instances of this task are created so the task parameter is used
	to pass in a constant that indicates whether this task is controlling
	the left side or right side display.  The constant is used as an index
	into the arrays defined at file scope within this file. */
	ucIndex = ( unsigned char ) pvParameters;
	
	/* A binary semaphore is used to signal button push events.  Create the
	semaphore before it is used. */
	vSemaphoreCreateBinary( xSemaphores[ ucIndex ] );

	/* Make sure the semaphore starts in the wanted state - no button pushes 
	pending. This call will just clear any button pushes that are latched.
	Passing in 0 as the block time means the call will not wait for any further
	button pushes but instead return immediately. */
	prvButtonHit( ucIndex, 0 );

	/* Seed the random number generator. */
	srand( ( unsigned char ) diceSHAKE_TIME );




	/* Start the task proper.  A loop will be performed each time a button is
	pushed.  The task will remain in the blocked state (sleeping) until a 
	button is pushed. */
	for( ;; )
	{
		/* Wait for a button push.  This task will enter the Blocked state
		(will not run again) until after a button has been pushed. */
		prvButtonHit( ucIndex, portMAX_DELAY );
		
		/* The next line will only execute after a button has been pushed -
		initialise the variable used to control the time the dice is shaken
		for. */
		ulDiceRunTime = diceSHAKE_TIME;				

		/* Suspend the flash tasks so this task has exclusive access to the
		display. */
		vSuspendFlashTasks( ucIndex, pdTRUE );

		/* Clear the display and pause for a short time, before starting to
		shake. */
		*pucDisplayOutput[ ucIndex ] = 0xff;
		vTaskDelay( diceSHORT_PAUSE_BEFORE_SHAKE );

		/* Keep generating and displaying random numbers until the shake time
		expires. */
		while( ulDiceRunTime > 0 )
		{
			ulDiceRunTime--;

			/* Generate and display a random number. */
			ucDiceValue = rand() % 6 + 1;
			dice7SEG_Value( ucIndex ) = ( dice7SEG_Value( ucIndex ) | 0xf7 ) & cDisplaySegments[ ucIndex ][ ucDiceValue ];

			/* Block/sleep for a very short time before generating the next
			random number. */
			vTaskDelay( diceDELAY_BETWEEN_RANDOM_NUMBERS_ms );
		}



		/* Clear any button pushes that are pending because a button bounced, or
		was pressed while the dice were shaking.  Again a block time of zero is 
		used so the function does not wait for any pushes but instead returns
		immediately. */
		prvButtonHit( ucIndex, 0 );

		/* Delay for a short while to display the dice shake result.  Use a queue
		peek here instead of a vTaskDelay() allows the delay to be interrupted by
		a button push.  If a button is pressed xQueuePeek() will return but the
		button push will remain pending to be read again at the top of this for
		loop.  It is safe to uses a queue function on a semaphore handle as
		semaphores are implemented as macros that uses queues, so the two are 
		basically the same thing. */
		xQueuePeek( xSemaphores[ ucIndex ], NULL, diceDELAY_WHILE_DISPLAYING_RESULT );

		/* Clear the display then resume the tasks or co-routines that were using
		the segments of the display. */
		*pucDisplayOutput[ ucIndex ] = 0xff;
		vSuspendFlashTasks( ucIndex, pdFALSE );
	}
}
/*-----------------------------------------------------------*/

/* Handler for the SW2 button push interrupt. */
__interrupt void vExternalInt8Handler( void )
{
short sHigherPriorityTaskWoken = pdFALSE;

	/* Reset the interrupt. */
	EIRR1_ER8 = 0;

	/* Check the semaphore has been created before attempting to use it. */
	if( xSemaphores[ configLEFT_DISPLAY ] != NULL )
	{
		/* Send a message via the semaphore to the dice task that controls the
		left side display.  This will unblock the task if it is blocked waiting
		for a button push. */
		xSemaphoreGiveFromISR( xSemaphores[ configLEFT_DISPLAY ], &sHigherPriorityTaskWoken );
	}

	/* If sending the semaphore unblocked a task, and the unblocked task has a
	priority that is higher than the currently running task, then force a context
	switch. */
	if( sHigherPriorityTaskWoken != pdFALSE )
	{
		portYIELD_FROM_ISR();
	}
}
/*-----------------------------------------------------------*/

/* As per vExternalInt8Handler(), but for SW3 and the right side display. */
__interrupt void vExternalInt9Handler( void )
{
short sHigherPriorityTaskWoken = pdFALSE;

	/* Reset the interrupt. */
	EIRR1_ER9 = 0;

	if( xSemaphores[ configRIGHT_DISPLAY ] != NULL )
	{
		xSemaphoreGiveFromISR( xSemaphores[ configRIGHT_DISPLAY ], &sHigherPriorityTaskWoken );
	}

	if( sHigherPriorityTaskWoken != pdFALSE )
	{
		portYIELD_FROM_ISR();
	}
}







