/* SHA-1 Accelerator - System verilog implementation
   Date: 20-07-2019
   submitted by:
	Jishnu Murali Thampan - 762574, jishnu.thampan@stud.h-da.de
*/

package state_machine_definitions;

	enum logic [1:0] {__RESET = 2'b00, __IDLE = 2'b01, __PROC = 2'b10, __DONE = 2'b11} state;

	// ...
	
endpackage

module state_machine_toplevel_framework(
	input logic clk, input logic reset_n, input logic start,
	output logic q_done,
	output logic [31:0]q_output_reg [4:0]
	);
	
	import state_machine_definitions::*;
	
	localparam LOOP_ITERATIONS = 120;
	
	localparam ITERATIONS = LOOP_ITERATIONS - 1;
	localparam BITWIDTH   = $clog2(ITERATIONS);
	
	
		/* ### start detection ... ################################################
	
						  _______________
			__________|					  |___________
			
						 ^
						 |
						
					  START
	
	*/
	
	logic [3:0] sync_reg = 4'b0000;
	
	/*Edge detection block*/
	always_ff@(posedge clk)
		begin : start_detection
			if(reset_n == 1'b0)
				sync_reg <= 4'b0000;
			else
				sync_reg <= {sync_reg[2:0],start};
		end : start_detection
			
	logic sync_start;
	assign sync_start = (sync_reg == 4'b0011) ? 1'b1 : 1'b0;
	
	logic [BITWIDTH-1:0] state_counter = 'd0;
	
	logic [1:0] ctrl = 2'd0;
	
	static int i = 0; /* For counting SHA-1 iterations*/
	
	/*SHA-1 constants*/
	logic [31:0] Hf0 = 32'h67452301;
	logic [31:0] Hf1 = 32'hefcdab89;
	logic [31:0] Hf2 = 32'h98badcfe;
	logic [31:0] Hf3 = 32'h10325476;
	logic [31:0] Hf4 = 32'hc3d2e1f0;
	
	/* For Preprocessor Stage*/
	localparam inputLength = 3;
	logic[(inputLength * 8) - 1:0] input_message = "abc";
	localparam OUTPUT_BITWIDTH = 512;
			
	/* The output of the pre-processing stage is stored here*/
	logic[OUTPUT_BITWIDTH:0] p_message32_16;
		
	/* For Conversion stage*/
	logic[31:0] Word[0:15];

	/* For compression stage */
	logic [31:0] Wt;
	logic [31:0] temp;
	logic [31:0] f,k;
	
	/*SHA-1 constants*/
	logic[31:0] H0 = 32'h67452301;
	logic[31:0] H1 = 32'hefcdab89;
	logic[31:0] H2 = 32'h98badcfe;
	logic[31:0] H3 = 32'h10325476;
	logic[31:0] H4 = 32'hc3d2e1f0;
		
	logic [31:0] K0 = 32'h5a827999;
	logic [31:0] K1 = 32'h6ed9eba1;
	logic [31:0] K2 = 32'h8f1bbcdc;
	logic [31:0] K3 = 32'hca62c1d6;
	
	logic[31:0] Wtemp;

	logic[31:0] W[15:0];
	logic [31:0] A = 32'h67452301;
	logic [31:0] B = 32'hefcdab89;
	logic [31:0] C = 32'h98badcfe;
	logic [31:0] D = 32'h10325476;
	logic [31:0] E = 32'hc3d2e1f0;	
	logic [OUTPUT_BITWIDTH-1:0] Message_Digest;
	 
	/* Function to perform rotation to the left*/
	function logic [31:0] rotate_left(int count, logic [31:0] data);
		for (int j = 0; j < count; j = j + 1) begin
			data = { data[30:0],data[31] };
    		end
		return data;
	endfunction
	
	/* States used for the SHA-processing*/
	enum logic [1:0] {SHA_IDLE = 2'b00, SHA_PRE_PROCESSING = 2'b01, SHA_CORE = 2'b10, SHA_DONE = 2'b11} sha_state;
	
	logic trigger = 1'd0; /* The central control path state machine triggers this to initiate the hash computation*/
	
	/* State machine to perform different stages in SHA-1 computation*/
	always_ff@(posedge clk)
	 begin:exec_state_machine
			if(reset_n == 0)
				sha_state <= SHA_IDLE;
				
			case(sha_state)
				SHA_IDLE: /* Represents the IDLE State */
					begin
						if(trigger == 1'd1) // this is set from the central state machine
							sha_state <= SHA_PRE_PROCESSING;
					end
							
				SHA_PRE_PROCESSING: /* Represents the SHA Pre-processing State */
					begin
						preProcessing();
						sha_state <= SHA_CORE;
					end
				
				SHA_CORE: /* Represents the SHA-Kernel*/
					begin
						if(SHA_Accelerator() == 1)
							sha_state <= SHA_DONE;
					end
				
				SHA_DONE: /* Represents the Final state where the hashes are acuumulated and output is available */
					begin
						sha_state <= SHA_IDLE;
					end
					
				endcase
	 end:exec_state_machine
	
	/* Function which performs SHA-Preprocessing*/
	function void 	preProcessing();
		p_message32_16[OUTPUT_BITWIDTH:OUTPUT_BITWIDTH-(inputLength * 8)] = input_message;
		p_message32_16[OUTPUT_BITWIDTH-((inputLength * 8) + 1)] = 1'b1; //append 1 to the end of the msg
		
		for(int i  = (OUTPUT_BITWIDTH - ((inputLength * 8) + 2)); i > 63; i-- )
		begin
			p_message32_16[i] = 1'b0; // fill zeros
		end
		
		p_message32_16[63:0] = inputLength * 8; // length of the array
	
		/* Conversion Stage*/
		for(int i = 1; i < 17; i++) 
		begin
			Word[16-i] = p_message32_16[(i * 32)-1 -:32];
		end
		
	endfunction
	
	/* Performs compression and intermediate hash accumulation
	   Algorithm part- the c code is directly mapped here*/
	function bit SHA_Accelerator();
							
      /* Compression stage*/							
		if(i >= 0 && i < 16)
		begin
			Wt   = Word[i];
			W[i] = Word[i];
		end
	
		else if(i >= 16 && i < 80)
		begin
			Wtemp = rotate_left(1, (W[0]^W[2]^W[8]^W[13]));
		
			for(int j = 0; j < 16; j++)
			begin
				if(j < 15) 
					W[j] = W[j+1];
				else
					W[j] = Wtemp;
				end
		
				Wt =  Wtemp;
		   end
			
		
		if(i >= 0 && i < 80)
		begin
			if(i >= 0 && i < 20)
			begin
				f = ((B & C) | ((~B) & D));
				k =  K0;
			end
			else if(i >= 20 && i < 40)
			begin
				f = (B ^ C ^ D);
				k    =  K1;    		
			end
			else if(i >= 40 && i < 60)
			begin
				f = ((B & C) | (B & D) | (C & D));
				k    =  K2;    		
			end
			else if(i >= 60 && i < 80)
			begin
				f = (B ^ C ^ D);
				k =  K3;
			end
			else
			begin
	
			end
			
			temp = rotate_left(5, A) + f + E + k + Wt;
			E = D;
			D = C;
			C = rotate_left(30, B);
			B = A;
			A = temp;
			
		end

		Hf0 = H0 + A;
		Hf1 = H1 + B;
		Hf2 = H2 + C;
		Hf3 = H3 + D;
		Hf4 = H4 + E;
		Message_Digest = (Hf0 << 128) | (Hf1 << 96) | (Hf2 << 64) | (Hf3 << 32) | Hf4;
		i++;
		
		if(i== 80)
			return 1;
		else
			return 0;
			
	endfunction
	
	// ### 'Central state machine' ... ################################################
		
	always_ff@(posedge clk) 
		begin : state_machine
			if(reset_n == 1'b0)
				begin
					ctrl          <= 2'd0;
					state_counter <=  'd0;
					state			  <= __RESET;
					q_done <= 1'b0;
				end
			else
				case(state)
					__RESET: begin
					
						ctrl          <= 2'd0;
						state_counter <=  'd0;
						state 		  <= __IDLE;
						q_done 		  <= 1'b0;
					end 
					__IDLE:  begin
					
						ctrl          <= 2'd0;
						state_counter <=  'd0;
						
						if(sync_start)begin
							state  <= __PROC;
							q_done <= 1'b0; // Reset previous output if any
						end
					end
					__PROC:  begin

						trigger <= 1'd1;/* Start processing here */
						
						if(state_counter < 10)
							ctrl <= 2'b00;
						else if(state_counter >= 10 && state_counter < 64)
							ctrl <= 2'b01;
						else if(state_counter >= 64 && state_counter < 110)
							ctrl <= 2'b10;
						else
							ctrl <= 2'b11;				
						
						
						if(sha_state == SHA_DONE)
							begin
								$display("Hash value of %s = %x",input_message,Message_Digest);
								/* Populate the output registers*/
								q_output_reg[0] <= Message_Digest[31:0];
								q_output_reg[1] <= Message_Digest[63:32];
								q_output_reg[2] <= Message_Digest[95:64];
								q_output_reg[3] <= Message_Digest[127:96];
								q_output_reg[4] <= Message_Digest[159:128];
								state         = __DONE;
								trigger <= 1'd0; /* End further processing*/
							end	
						
						else if(state_counter == ITERATIONS)
							begin
								state_counter <= 'd0;
								state         <= __DONE;
							end
						
						else
							begin
								state_counter++;
								state         <= __PROC;
							end
						
						end
							
					__DONE:  begin
						ctrl          <= 0;
						state_counter <= 0;
						q_done <= 1'b1;
						state 		  <= __IDLE;
					end
					
					default: begin
						ctrl          <= 0;
						state_counter <= 0;
						state 		  <= __RESET;
					end
				endcase	
		end : state_machine

endmodule 
