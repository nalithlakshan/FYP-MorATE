module IO_driver(
	input		      s_clk,
	input			  c_clk,
	input             reset,
	//SPI related 
	input 		      MOSI,
	output            MISO,
	input		      SCK,
	input [1:0]	      CS_addr,

	input [3:0]  	  addr,          // Address of a io pin driver (source fifo + capture fifo)
	input [2:0]		  request,       //1-spi_wrreq, 2-spi_rdreq, 3-copy_from_spi_req, 4-copy_to_spi_req, 5-exe_start, 6-clock_output
	input 			  process_rqst,
	input [15:0] 	  io_capture_pins,
	output[15:0]      io_source_pins,

	output 	 req_done,     // indicate that a copy operation is done

	//for testing
	output [1:0] state_indication,
	output s_clock_probe,
	output c_clock_probe
);
// wire clk1;
// reg [5:0] clock_div = 0;
// always @ (posedge clk)begin
// 	clock_div <= clock_div +1;
// end
// assign clk1 = clock_div[0];
// assign clk1 = s_clk;
assign s_clock_probe = s_clk;
assign c_clock_probe = c_clk;


parameter FPGA_ADDR = 2;
wire CS;
assign CS = (CS_addr == FPGA_ADDR)? 1:0;
//tri state buffers for outputs to MCU
reg	 req_done_flag;
wire MISO_bufin;
bufif1 req_done_buf(req_done, req_done_flag, CS);
bufif1 MISO_buf(MISO,MISO_bufin,CS); 

//Chip Selected signals (ie. to make CS the FPGA enable)
wire SCK_CS;
assign SCK_CS = (CS)? SCK : 0;

wire spi_wrreq_CS;
wire spi_rdreq_CS;
reg copy_from_spi_req_CS;
reg copy_to_spi_req_CS;
reg exe_start;


assign spi_wrreq_CS  = (CS && (request == 1))? 1 : 0;
assign spi_rdreq_CS  = (CS && (request == 2))? 1 : 0;


//for execution start condition
reg [15:0] exe_start_flags;
reg [15:0] capture_start_flags;

//connection to clear all FIFOs
wire aclr;
assign aclr = ~reset;

//multiplexers for clock outputs
reg[15:0] io_clk_source_en;
wire[15:0] source_fifo_outputs;

assign io_source_pins[0] = (io_clk_source_en[0])? s_clk :source_fifo_outputs[0];
assign io_source_pins[1] = (io_clk_source_en[1])? s_clk :source_fifo_outputs[1];
assign io_source_pins[2] = (io_clk_source_en[2])? s_clk :source_fifo_outputs[2];
assign io_source_pins[3] = (io_clk_source_en[3])? s_clk :source_fifo_outputs[3];
assign io_source_pins[4] = (io_clk_source_en[4])? s_clk :source_fifo_outputs[4];
assign io_source_pins[5] = (io_clk_source_en[5])? s_clk :source_fifo_outputs[5];
assign io_source_pins[6] = (io_clk_source_en[6])? s_clk :source_fifo_outputs[6];
assign io_source_pins[7] = (io_clk_source_en[7])? s_clk :source_fifo_outputs[7];
assign io_source_pins[8] = (io_clk_source_en[8])? s_clk :source_fifo_outputs[8];
assign io_source_pins[9] = (io_clk_source_en[9])? s_clk :source_fifo_outputs[9];
assign io_source_pins[10] = (io_clk_source_en[10])? s_clk :source_fifo_outputs[10];
assign io_source_pins[11] = (io_clk_source_en[11])? s_clk :source_fifo_outputs[11];
assign io_source_pins[12] = (io_clk_source_en[12])? s_clk :source_fifo_outputs[12];
assign io_source_pins[13] = (io_clk_source_en[13])? s_clk :source_fifo_outputs[13];
assign io_source_pins[14] = (io_clk_source_en[14])? s_clk :source_fifo_outputs[14];
assign io_source_pins[15] = (io_clk_source_en[15])? s_clk :source_fifo_outputs[15];


/*##################### MOSI FIFO to SOURCE FIFOs  ######################*/
//spi_mosi_fifo_connections
wire spi_mosi_fifo_rdempty;
wire spi_mosi_fifo_rdreq;
wire spi_mosi_fifo_q;
reg copy_from_spi_flag;
assign spi_mosi_fifo_rdreq = copy_from_spi_flag & (~spi_mosi_fifo_rdempty);

//source_fifo1 to source_fifo16 connections
reg [15:0] source_fifo_wrreq;
wire[15:0] source_fifo_rdreq;
wire[15:0] source_fifo_rdempty;
assign source_fifo_rdreq = exe_start_flags & (~source_fifo_rdempty);

/*##################### CAPTURE FIFOs to MISO FIFO  #####################*/
//spi_miso_fifo_connections
reg spi_miso_fifo_data;
reg spi_miso_fifo_wrreq;
reg copy_to_spi_flag;

//capture_fifo1 to capture_fifo16 connections
wire [15:0] capture_fifo_rdreq;
wire [15:0] capture_fifo_empty;
wire [15:0] capture_fifo_q;
wire 		capture_fifo_wrreq;
assign capture_fifo_wrreq = (capture_start_flags == 0)? 0:1;

//state variable
reg [1:0] state;
assign state_indication = state;

/*################ MULTIPLEXERS and DEMULTIPLEXERS ######################*/

//source_fifo write request DEMUX
// assign source_fifo_wrreq[0]  = (addr == 0)? spi_mosi_fifo_rdreq : 0; 
// assign source_fifo_wrreq[1]  = (addr == 1)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[2]  = (addr == 2)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[3]  = (addr == 3)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[4]  = (addr == 4)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[5]  = (addr == 5)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[6]  = (addr == 6)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[7]  = (addr == 7)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[8]  = (addr == 8)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[9]  = (addr == 9)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[10] = (addr ==10)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[11] = (addr ==11)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[12] = (addr ==12)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[13] = (addr ==13)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[14] = (addr ==14)? spi_mosi_fifo_rdreq : 0;
// assign source_fifo_wrreq[15] = (addr ==15)? spi_mosi_fifo_rdreq : 0;

//spi_miso_fifo_wrreq signal
// always @(*) begin
// 	case(addr)
// 		0:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[0]; 
// 		1:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[1];
// 		2:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[2];
// 		3:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[3];
// 		4:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[4];
// 		5:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[5];
// 		6:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[6];
// 		7:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[7];
// 		8:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[8];
// 		9:  spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[9];
// 		10: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[10];
// 		11: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[11];
// 		12: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[12];
// 		13: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[13];
// 		14: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[14];
// 		15: spi_miso_fifo_wrreq = copy_to_spi_flag & capture_fifo_empty[15];
// 	endcase
// end

//capture_fifo read request DEMUX
assign capture_fifo_rdreq[0]  = (addr == 0)? copy_to_spi_flag & (~capture_fifo_empty[0] ) : 0; 
assign capture_fifo_rdreq[1]  = (addr == 1)? copy_to_spi_flag & (~capture_fifo_empty[1] ) : 0;
assign capture_fifo_rdreq[2]  = (addr == 2)? copy_to_spi_flag & (~capture_fifo_empty[2] ) : 0;
assign capture_fifo_rdreq[3]  = (addr == 3)? copy_to_spi_flag & (~capture_fifo_empty[3] ) : 0;
assign capture_fifo_rdreq[4]  = (addr == 4)? copy_to_spi_flag & (~capture_fifo_empty[4] ) : 0;
assign capture_fifo_rdreq[5]  = (addr == 5)? copy_to_spi_flag & (~capture_fifo_empty[5] ) : 0;
assign capture_fifo_rdreq[6]  = (addr == 6)? copy_to_spi_flag & (~capture_fifo_empty[6] ) : 0;
assign capture_fifo_rdreq[7]  = (addr == 7)? copy_to_spi_flag & (~capture_fifo_empty[7] ) : 0;
assign capture_fifo_rdreq[8]  = (addr == 8)? copy_to_spi_flag & (~capture_fifo_empty[8] ) : 0;
assign capture_fifo_rdreq[9]  = (addr == 9)? copy_to_spi_flag & (~capture_fifo_empty[9] ) : 0;
assign capture_fifo_rdreq[10] = (addr ==10)? copy_to_spi_flag & (~capture_fifo_empty[10]) : 0;
assign capture_fifo_rdreq[11] = (addr ==11)? copy_to_spi_flag & (~capture_fifo_empty[11]) : 0;
assign capture_fifo_rdreq[12] = (addr ==12)? copy_to_spi_flag & (~capture_fifo_empty[12]) : 0;
assign capture_fifo_rdreq[13] = (addr ==13)? copy_to_spi_flag & (~capture_fifo_empty[13]) : 0;
assign capture_fifo_rdreq[14] = (addr ==14)? copy_to_spi_flag & (~capture_fifo_empty[14]) : 0;
assign capture_fifo_rdreq[15] = (addr ==15)? copy_to_spi_flag & (~capture_fifo_empty[15]) : 0;

//assign cature_fifo_q to spi_miso_fifo_data
always @(*) begin
	case(addr)
		0:  spi_miso_fifo_data = capture_fifo_q[0]; 
		1:  spi_miso_fifo_data = capture_fifo_q[1];
		2:  spi_miso_fifo_data = capture_fifo_q[2];
		3:  spi_miso_fifo_data = capture_fifo_q[3];
		4:  spi_miso_fifo_data = capture_fifo_q[4];
		5:  spi_miso_fifo_data = capture_fifo_q[5];
		6:  spi_miso_fifo_data = capture_fifo_q[6];
		7:  spi_miso_fifo_data = capture_fifo_q[7];
		8:  spi_miso_fifo_data = capture_fifo_q[8];
		9:  spi_miso_fifo_data = capture_fifo_q[9];
		10: spi_miso_fifo_data = capture_fifo_q[10];
		11: spi_miso_fifo_data = capture_fifo_q[11];
		12: spi_miso_fifo_data = capture_fifo_q[12];
		13: spi_miso_fifo_data = capture_fifo_q[13];
		14: spi_miso_fifo_data = capture_fifo_q[14];
		15: spi_miso_fifo_data = capture_fifo_q[15];
	endcase
end


//STATES
parameter IDLE_OR_SPI	      = 0;
parameter COPY_FROM_MOSI_FIFO = 1;
parameter COPY_TO_MISO_FIFO	  = 2;
parameter EXECUTION 	      = 3;

always @(posedge s_clk or negedge reset) begin
	if(~reset) begin
		//resets
		exe_start_flags    <= 0;
		capture_start_flags<= 0;
		req_done_flag      <= 1;
		copy_from_spi_flag <= 0;
		copy_to_spi_flag   <= 0;
		state              <= 0;
		source_fifo_wrreq  <= 0;
		spi_miso_fifo_wrreq<= 0;
		io_clk_source_en   <= 0;
	end 

	else begin
		source_fifo_wrreq  <= (1&spi_mosi_fifo_rdreq)<<addr;

		spi_miso_fifo_wrreq <= 1&(capture_fifo_rdreq == (1<<addr));

		capture_start_flags <= source_fifo_rdreq;

		copy_from_spi_req_CS <= (CS && (request == 3) &&(process_rqst))? 1 : 0;
		copy_to_spi_req_CS   <= (CS && (request == 4) &&(process_rqst))? 1 : 0;
		exe_start            <= ((request == 5) &&(process_rqst))? 1 : 0;

		if(CS && (request == 6) &&(process_rqst))begin
			io_clk_source_en <= (io_clk_source_en | (1<<addr));
		end
		// else begin
			// io_clk_source_en <= io_clk_source_en & (~(16'b1111111111111111&(spi_mosi_fifo_rdreq<<addr)));
		// end

		case(state)
			IDLE_OR_SPI: begin
				if(copy_from_spi_req_CS)begin
					copy_from_spi_flag <= 1;
					copy_to_spi_flag   <= 0;
					req_done_flag	   <= 0;
					exe_start_flags	   <= 0;
					state 			   <= COPY_FROM_MOSI_FIFO;
				end
				else if(copy_to_spi_req_CS)begin
					copy_from_spi_flag <= 0;
					copy_to_spi_flag   <= 1;
					req_done_flag	   <= 0;
					exe_start_flags	   <= 0;
					state 			   <= COPY_TO_MISO_FIFO;
				end
				else if(exe_start)begin
					copy_from_spi_flag <= 0;
					copy_to_spi_flag   <= 0;
					exe_start_flags	   <= ~source_fifo_rdempty;
					req_done_flag	   <= 0;
					state 			   <= EXECUTION;
				end
				else begin
					copy_from_spi_flag <= 0;
					copy_to_spi_flag   <= 0;
					exe_start_flags	   <= 0;
					state 			   <= IDLE_OR_SPI;
				end
			end

			COPY_FROM_MOSI_FIFO: begin
				if(spi_mosi_fifo_rdempty)begin
					copy_from_spi_flag <= 0;
					req_done_flag	   <= 1;
					state 			   <= IDLE_OR_SPI;
				end
			end

			COPY_TO_MISO_FIFO: begin
				if(capture_fifo_rdreq == 0)begin
					copy_to_spi_flag   <= 0;
					req_done_flag	   <= 1;
					state 			   <= IDLE_OR_SPI;
				end
			end

			EXECUTION: begin
				exe_start_flags	       <= ~source_fifo_rdempty;
				if(exe_start_flags == 0)begin
					req_done_flag      <= 1;
					state 			   <= IDLE_OR_SPI;    
				end
			end

		endcase
	end
end


/*######################  Instantiations #########################*/
fifo_2clk spi_mosi_fifo(
	.aclr   (aclr),
	.data   (MOSI),
	.rdclk  (s_clk),
	.rdreq  (spi_mosi_fifo_rdreq),
	.wrclk  (SCK_CS),
	.wrreq  (spi_wrreq_CS),
	.q      (spi_mosi_fifo_q),
	.rdempty(spi_mosi_fifo_rdempty)
);


fifo_2clk spi_miso_fifo(
	.aclr   (aclr),
	.data   (spi_miso_fifo_data),
	.rdclk  (SCK_CS),
	.rdreq  (spi_rdreq_CS),
	.wrclk  (s_clk),
	.wrreq  (spi_miso_fifo_wrreq),
	.q      (MISO_bufin)
);


genvar k;
generate
   for (k=0; k<16; k=k+1) begin : generate_source_fifo_instances
   fifo_1clk source_fifo(
   	.aclr (aclr),
   	.clock(s_clk),
   	.data (spi_mosi_fifo_q),
   	.rdreq(source_fifo_rdreq[k]),
   	.wrreq(source_fifo_wrreq[k]),
   	.empty(source_fifo_rdempty[k]),
   	.q    (source_fifo_outputs[k]) 
	);
   end 
endgenerate

genvar j;
generate
   for (j=0; j<16; j=j+1) begin : generate_capture_fifo_instances
   fifo_1clk capture_fifo(
   	.aclr (aclr),
   	.clock(s_clk),
   	.data (io_capture_pins[j]),
   	.rdreq(capture_fifo_rdreq[j]),
   	.wrreq(capture_fifo_wrreq),
   	.empty(capture_fifo_empty[j]),
   	.q    (capture_fifo_q[j])
	);
 //   fifo_2clk capture_fifo(
	// .aclr   (aclr),
	// .data   (io_capture_pins[j]),
	// .rdclk  (s_clk),
	// .rdreq  (capture_fifo_rdreq[j]),
	// .wrclk  (c_clk),
	// .wrreq  (capture_fifo_wrreq),
	// .q      (capture_fifo_q[j]),
	// .rdempty(capture_fifo_empty[j])
 //    );
   end 
endgenerate

endmodule