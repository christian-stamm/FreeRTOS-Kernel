/*
 * FreeRTOS+CLI V1.0.3 (C) 2014 Real Time Engineers ltd. All rights reserved.
 *
 * This file is part of the FreeRTOS+CLI distribution.  The FreeRTOS+CLI license
 * terms are different to the FreeRTOS license terms.
 *
 * FreeRTOS+CLI uses a dual license model that allows the software to be used
 * under a standard GPL open source license, or a commercial license.  The
 * standard GPL license (unlike the modified GPL license under which FreeRTOS
 * itself is distributed) requires that all software statically linked with
 * FreeRTOS+CLI is also distributed under the same GPL V2 license terms.
 * Details of both license options follow:
 *
 * - Open source licensing -
 * FreeRTOS+CLI is a free download and may be used, modified, evaluated and
 * distributed without charge provided the user adheres to version two of the
 * GNU General Public License (GPL) and does not remove the copyright notice or
 * this text.  The GPL V2 text is available on the gnu.org web site, and on the
 * following URL: http://www.FreeRTOS.org/gpl-2.0.txt.
 *
 * - Commercial licensing -
 * Businesses and individuals that for commercial or other reasons cannot comply
 * with the terms of the GPL V2 license must obtain a low cost commercial
 * license before incorporating FreeRTOS+CLI into proprietary software for
 * distribution in any form.  Commercial licenses can be purchased from
 * http://shop.freertos.org/cli and do not require any source files to be
 * changed.
 *
 * FreeRTOS+CLI is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+CLI unless you agree that you use the software 'as is'.
 * FreeRTOS+CLI is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/FreeRTOS-Plus
 *
 */

#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H

/* The prototype to which callback functions used to process command line
commands must comply.  pcWriteBuffer is a buffer into which the output from
executing the command can be written, xWriteBufferLen is the length, in bytes of
the pcWriteBuffer buffer, and pcCommandString is the entire string as input by
the user (from which parameters can be extracted).*/
typedef portBASE_TYPE (*pdCOMMAND_LINE_CALLBACK)( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/* The structure that defines command line commands.  A command line command
should be defined by declaring a const structure of this type. */
typedef struct xCOMMAND_LINE_INPUT
{
	const char * const pcCommand;				/* The command that causes pxCommandInterpreter to be executed.  For example "help".  Must be all lower case. */
	const char * const pcHelpString;			/* String that describes how to use the command.  Should start with the command itself, and end with "\r\n".  For example "help: Returns a list of all the commands\r\n". */
	const pdCOMMAND_LINE_CALLBACK pxCommandInterpreter;	/* A pointer to the callback function that will return the output generated by the command. */
	int8_t cExpectedNumberOfParameters;			/* Commands expect a fixed number of parameters, which may be zero. */
} CLI_Command_Definition_t;

/* For backward compatibility. */
#define xCommandLineInput CLI_Command_Definition_t

/*
 * Register the command passed in using the pxCommandToRegister parameter.
 * Registering a command adds the command to the list of commands that are
 * handled by the command interpreter.  Once a command has been registered it
 * can be executed from the command line.
 */
portBASE_TYPE FreeRTOS_CLIRegisterCommand( const CLI_Command_Definition_t * const pxCommandToRegister );

/*
 * Runs the command interpreter for the command string "pcCommandInput".  Any
 * output generated by running the command will be placed into pcWriteBuffer.
 * xWriteBufferLen must indicate the size, in bytes, of the buffer pointed to
 * by pcWriteBuffer.
 *
 * FreeRTOS_CLIProcessCommand should be called repeatedly until it returns pdFALSE.
 *
 * pcCmdIntProcessCommand is not reentrant.  It must not be called from more
 * than one task - or at least - by more than one task at a time.
 */
portBASE_TYPE FreeRTOS_CLIProcessCommand( const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen  );

/*-----------------------------------------------------------*/

/*
 * A buffer into which command outputs can be written is declared in the
 * main command interpreter, rather than in the command console implementation,
 * to allow application that provide access to the command console via multiple
 * interfaces to share a buffer, and therefore save RAM.  Note, however, that
 * the command interpreter itself is not re-entrant, so only one command
 * console interface can be used at any one time.  For that reason, no attempt
 * is made to provide any mutual exclusion mechanism on the output buffer.
 *
 * FreeRTOS_CLIGetOutputBuffer() returns the address of the output buffer.
 */
char *FreeRTOS_CLIGetOutputBuffer( void );

/*
 * Return a pointer to the xParameterNumber'th word in pcCommandString.
 */
const char *FreeRTOS_CLIGetParameter( const char *pcCommandString, unsigned portBASE_TYPE uxWantedParameter, portBASE_TYPE *pxParameterStringLength );

#endif /* COMMAND_INTERPRETER_H */













