/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */
/* SHA-1 Accelerator - C implementation
   Date: 20-07-2019
   submitted by:
   Jishnu Murali Thampan - 762574, jishnu.thampan@stud.h-da.de
   George Sebastian - 762610, george.sebastian@stud.h-da.de
*/
#include "sys/alt_stdio.h"
#include "system.h"
#include "sha-1.h"
#include <sys/time.h>

#define CRYPTO_ACCELERATOR_ENABLED /* Enables HW Accelerator */
#define CRYPTO_CTRL_REG (0)    /* Represents the control register address offset*/
#define CRYPTO_STATUS_REG (1) /* Represents the status register address offset*/

#define CRYPTO_DATA_REG_0 (2)  /* Represents the data register-0 address offset*/
#define CRYPTO_DATA_REG_1 (3)  /* Represents the data register-1 address offset*/
#define CRYPTO_DATA_REG_2 (4)  /* Represents the data register-2 address offset*/
#define CRYPTO_DATA_REG_3 (5)  /* Represents the data register-3 address offset*/
#define CRYPTO_DATA_REG_4 (6)  /* Represents the data register-4 address offset*/

#define CRYPTO_CTRL_REG_BIT_0 (0) /* To start processing*/
#define CRYPTO_STATUS_REG_BIT_0 (0) /* Represents that output is ready from the HW */


/* Function checks if the computed hash matches with the expected hash*/
int isMatched(const uint32_t* expectedHash, const uint32_t* actualHash)
{
	int count = 0;
	for (; count < FINAL_HASH_SIZE; count++)
	 {
		if(actualHash[count] != expectedHash[count])
	 	{
			break;
	 	}
	  }
	return (count == FINAL_HASH_SIZE) ? (1) : (0);
}

int main()
{ 
  volatile unsigned int * led_ptr = (volatile unsigned int *)0x80009040;
  alt_putstr("Hello from Nios II!\n");

  uint32_t expectedHash[FINAL_HASH_SIZE]={};
  expectedHash[0] = 2845392438;
  expectedHash[1] = 1191608682;
  expectedHash[2] = 3124634993;
  expectedHash[3] = 2018558572;
  expectedHash[4] = 2630932637;

  struct timeval start, end;

#ifndef CRYPTO_ACCELERATOR_ENABLED /* Software computation */

  uint32_t final_hash[FINAL_HASH_SIZE] = { 0 };
  gettimeofday(&start, NULL);
  generate_sha1_hash(final_hash); // Call the SHA-1 in software
  gettimeofday(&end, NULL);

  /* Print the results to the console */
  printf("Software: Time taken[in uS]=%ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
		  - (start.tv_sec * 1000000 + start.tv_usec)));

  /* Turn on the led if the output is correct */
  if(isMatched(expectedHash, final_hash))
  {
	  *led_ptr = 0xFF;
  }
  /* Event loop never exits. */
  while (1);
#endif

#ifdef CRYPTO_ACCELERATOR_ENABLED /* HW computation */
  volatile unsigned int *crypto_engine_ptr = (volatile unsigned int *)0x80009000;
  *(crypto_engine_ptr + CRYPTO_CTRL_REG) = (1 << CRYPTO_CTRL_REG_BIT_0); //set bit 0 to start
  gettimeofday(&start, NULL); //start system timer
  while(*(crypto_engine_ptr + CRYPTO_STATUS_REG) == (1 << CRYPTO_STATUS_REG_BIT_0));
  gettimeofday(&end, NULL); //end system timer

  printf("Hardware: Time taken[in uS]=%ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
		  - (start.tv_sec * 1000000 + start.tv_usec)));

  /* Get the output from HW data registers */
  unsigned int sha_engine_output[FINAL_HASH_SIZE]={};
  sha_engine_output[4] = *(crypto_engine_ptr + CRYPTO_DATA_REG_0);
  sha_engine_output[3] = *(crypto_engine_ptr + CRYPTO_DATA_REG_1);
  sha_engine_output[2] = *(crypto_engine_ptr + CRYPTO_DATA_REG_2);
  sha_engine_output[1] = *(crypto_engine_ptr + CRYPTO_DATA_REG_3);
  sha_engine_output[0] = *(crypto_engine_ptr + CRYPTO_DATA_REG_4);

  if(isMatched(expectedHash, sha_engine_output))
  {
	  *led_ptr = 0xFF; /* Turn on the led if the output is correct */
  }
  print_final_hash(sha_engine_output); /* Print the results to the console */

  while(1);

#endif
  return 0;
}
