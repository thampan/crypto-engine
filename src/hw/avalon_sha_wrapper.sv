/* SHA-1 Accelerator - System verilog implementation
   Date: 20-07-2019
   submitted by:
	Jishnu Murali Thampan
*/

/* Register Usage Information:
	Reg0 - Control register => bit0 to enable, bit [31:1] = unused
    Reg1 - Status register  => processing done => bit0 is set
	Reg2 - Output Data register
	.   
	.
	Reg6
	Reg7 - Unused */
	
 module avalon_sha_wrapper(
	input logic clk, input logic reset_n,
	input logic read, input logic write,
	input logic [2:0]   address,
	input logic [31:0]  writedata,
	output logic [31:0] readdata
	);
	
	logic [31:0] control_register    = 0; /* Contains the enable bit set/reset */
	logic [31:0] status_register     = 0; /* Contains the status bit - done/not done */
	logic [31:0] data_register [4:0] = 0; /* Contains the output hash */

	logic q_done = 0 , start = 0;
	integer i;
	
	always_ff@(posedge clk) begin
		if(reset_n == 1'b0)
			begin
				for(i = 0;i <= 4; i = i+1) begin: clear_register_set
					data_register[i] <= 32'd0;
				end
				control_register <= 32'd0;
				status_register  <= 32'd0;
			end
		else
			if(write)
				case(address)
					0: control_register <= writedata;
					1: status_register  <= writedata;
					2: data_register[0] <= writedata;
					3: data_register[1] <= writedata;
					4: data_register[2] <= writedata;
					5: data_register[3] <= writedata;
					6:	data_register[4] <= writedata;
					7:	data_register[4] <= writedata; //unused
				endcase
				
	end
	always_comb
	begin
		if(read)
			case(address)
				0: readdata = control_register;
				1: readdata = status_register;
				2: readdata = data_register[0];
				3: readdata = data_register[1];
				4: readdata = data_register[2];
				5: readdata = data_register[3];
				6:	readdata = data_register[4];
				7: readdata = data_register[4];//unused
			endcase
		else
			readdata = 0;
	end
	
	assign start = (control_register[0] == 1) ? 1 : 0; // If BIT 0 is set, then start
	assign status_register = (q_done == 1) ? 1 : 0; // If processing is done, set bit 0
	
	state_machine_toplevel_framework inst_0(.clk(clk),
		.reset_n(reset_n),
	   .start(start),
		.q_done(q_done),
		.q_output_reg(data_register));

endmodule
