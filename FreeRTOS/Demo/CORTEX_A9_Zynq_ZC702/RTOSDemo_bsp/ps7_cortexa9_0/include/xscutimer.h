/******************************************************************************
*
* (c) Copyright 2010-12 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xscutimer.h
*
* The timer driver supports the Cortex A9 private timer.
*
* The timer driver supports the following features:
* - Normal mode and Auto reload mode
* - Interrupts (Interrupt handler is not provided in this driver. Application
* 		has to register it's own handler)
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate with the Timer.
*
* XScuTimer_CfgInitialize() API is used to initialize the Timer. The
* user needs to first call the XScuTimer_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to
* the XScuTimer_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The Timer hardware supports interrupts.
*
* This driver does not provide a Interrupt Service Routine (ISR) for the device.
* It is the responsibility of the application to provide one if needed. Refer to
* the interrupt example provided with this driver for details on using the
* Timer in interrupt mode.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XScuTimer driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <br><br>
*
* NOTE:
* The timer is not a part of the snoop control unit as indicated by the
* prefix "scu" in the name of the driver.
* It is an independent module in APU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a nm  03/10/10 First release
* 1.02a sg  07/17/12 Included xil_assert.h for CR 667947. This is an issue
*		     when the xstatus.h in the common driver overwrites
*		     the xstatus.h of the standalone BSP during the
*		     libgen.
* </pre>
*
******************************************************************************/
#ifndef XSCUTIMER_H		/* prevent circular inclusions */
#define XSCUTIMER_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xscutimer_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;	/**< Unique ID of device */
	u32 BaseAddr;	/**< Base address of the device */
} XScuTimer_Config;

/**
 * The XScuTimer driver instance data. The user is required to allocate a
 * variable of this type for every timer device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XScuTimer_Config Config; /**< Hardware Configuration */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 IsStarted;		/**< Device timer is running */
} XScuTimer;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Check if the timer has expired.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return
*		- TRUE if the timer has expired.
*		- FALSE if the timer has not expired.
*
* @note		C-style signature:
*		int XScuTimer_IsExpired(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_IsExpired(InstancePtr) \
	((XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
				XSCUTIMER_ISR_OFFSET) & \
				XSCUTIMER_ISR_EVENT_FLAG_MASK) == \
				XSCUTIMER_ISR_EVENT_FLAG_MASK)

/****************************************************************************/
/**
*
* Re-start the timer. This macro will read the timer load register
* and writes the same value to load register to update the counter register.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_RestartTimer(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_RestartTimer(InstancePtr)				\
	XScuTimer_LoadTimer(InstancePtr,				\
		XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
					XSCUTIMER_LOAD_OFFSET))

/****************************************************************************/
/**
*
* Write to the timer load register. This will also update the
* timer counter register with the new value. This macro can be used to
* change the time-out value.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
* @param	Value is the count to be loaded in to the load register.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_LoadTimer(XScuTimer *InstancePtr, u32 Value)
*
******************************************************************************/
#define XScuTimer_LoadTimer(InstancePtr, Value)				\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_LOAD_OFFSET, Value)

/****************************************************************************/
/**
*
* Returns the current timer counter register value. It can be called at any
* time.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	Contents of the timer counter register.
*
* @note		C-style signature:
		u32 XScuTimer_GetCounterValue(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_GetCounterValue(InstancePtr)				\
	XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr,		\
				XSCUTIMER_COUNTER_OFFSET)

/****************************************************************************/
/**
*
* Enable auto-reload mode.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_EnableAutoReload(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_EnableAutoReload(InstancePtr)				\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_CONTROL_OFFSET,			\
			(XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
				XSCUTIMER_CONTROL_OFFSET) |		 \
				XSCUTIMER_CONTROL_AUTO_RELOAD_MASK))

/****************************************************************************/
/**
*
* Disable auto-reload mode.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_DisableAutoReload(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_DisableAutoReload(InstancePtr)			\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_CONTROL_OFFSET,			\
			(XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
				XSCUTIMER_CONTROL_OFFSET) &		\
				~(XSCUTIMER_CONTROL_AUTO_RELOAD_MASK)))

/****************************************************************************/
/**
*
* Enable the Timer interrupt.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_EnableInterrupt(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_EnableInterrupt(InstancePtr)				\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_CONTROL_OFFSET,			\
			(XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
					XSCUTIMER_CONTROL_OFFSET) |	\
					XSCUTIMER_CONTROL_IRQ_ENABLE_MASK))

/****************************************************************************/
/**
*
* Disable the Timer interrupt.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_DisableInterrupt(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_DisableInterrupt(InstancePtr)				\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_CONTROL_OFFSET,			\
			(XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr, \
				XSCUTIMER_CONTROL_OFFSET) &		\
				~(XSCUTIMER_CONTROL_IRQ_ENABLE_MASK)))

/*****************************************************************************/
/**
*
* This function reads the interrupt status.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_GetInterruptStatus(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_GetInterruptStatus(InstancePtr)			\
	XScuTimer_ReadReg((InstancePtr)->Config.BaseAddr,		\
			XSCUTIMER_ISR_OFFSET)

/*****************************************************************************/
/**
*
* This function clears the interrupt status.
*
* @param	InstancePtr is a pointer to the XScuTimer instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XScuTimer_ClearInterruptStatus(XScuTimer *InstancePtr)
*
******************************************************************************/
#define XScuTimer_ClearInterruptStatus(InstancePtr)			\
	XScuTimer_WriteReg((InstancePtr)->Config.BaseAddr,		\
		XSCUTIMER_ISR_OFFSET, XSCUTIMER_ISR_EVENT_FLAG_MASK)

/************************** Function Prototypes ******************************/

/*
 * Lookup configuration in xscutimer_sinit.c
 */
XScuTimer_Config *XScuTimer_LookupConfig(u16 DeviceId);

/*
 * Selftest function in xscutimer_selftest.c
 */
int XScuTimer_SelfTest(XScuTimer *InstancePtr);

/*
 * Interface functions in xscutimer.c
 */
int XScuTimer_CfgInitialize(XScuTimer *InstancePtr,
			    XScuTimer_Config *ConfigPtr, u32 EffectiveAddress);
void XScuTimer_Start(XScuTimer *InstancePtr);
void XScuTimer_Stop(XScuTimer *InstancePtr);
void XScuTimer_SetPrescaler(XScuTimer *InstancePtr, u8 PrescalerValue);
u8 XScuTimer_GetPrescaler(XScuTimer *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
