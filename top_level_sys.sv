/* SHA-1 Accelerator - System verilog implementation
   Date: 20-07-2019
   submitted by:
	Jishnu Murali Thampan - 762574, jishnu.thampan@stud.h-da.de
	George Sebastian - 762610, george.sebastian@stud.h-da.de
*/
module top_level_sys(
	(* chip_pin = "AF14" *) input logic clk,
	(* chip_pin =  "AJ4" *) input logic reset_n, 
	(* chip_pin = "AF24, AE24, AF25, AG25, AD24, AC23, AB23, AA24" *) output logic [7:0] leds,
		// DE10-Standard - Some GPIOs
	(* chip_pin =  "AK2, W15" *) output logic [1:0] gpio
	);		
	
	logic [7:0] nios_led_output; //unused for now
	
	// instantiate the Nios right here ..., use the upper signals to interface the onboard LEDs 

	base_sys u0 (
		.clk_clk         (clk),         //      clk.clk
		.pio_leds_export (nios_led_output), // pio_leds.export
		.reset_reset_n   (reset_n),   //    reset.reset_n
		.time_out_export (gpio)  // time_out.export
	);


	//assign leds = {8{nios_led_output}};
		
	
endmodule 