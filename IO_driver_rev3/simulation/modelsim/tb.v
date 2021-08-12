// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on
module  tb;

reg		      clk;
reg           reset;
//SPI related 
reg 	      MOSI;
wire          MISO;
reg		      SCK;
reg	[1:0]     CS;
reg 		  process_rqst;
// reg           spi_wrreq;
// reg           spi_rdreq;

reg [3:0]  	  addr;         // Address of a io pin driver (source fifo + capture fifo)
reg [2:0]     request;		 //1-spi_wrreq, 2-spi_rdreq, 3-copy_from_spi_req, 4-copy_to_spi_req, 5-exe_start
// reg        	  copy_from_spi_req;
// reg        	  copy_to_spi_req;
wire[15:0] 	  io_capture_pins;
wire[15:0]    io_source_pins;

// reg	          exe_start;	  // start execution of sinking and sourcing data streams
wire 	 	  req_done;    	  // indicate that a copy operation is done
wire[1:0] 	  state_indication;

//requests
parameter NO_REQ	    = 0;	
parameter SPI_WRREQ     = 1;
parameter SPI_RDREQ     = 2;
parameter COPY_FROM_SPI = 3;
parameter COPY_TO_SPI   = 4;
parameter EXE_START     = 5;


//Virtual IC to be tested
wire pin1; //5V            
wire pin2; //A1           <-- connect to io2  
wire pin3; //B1           <-- connect to io3 
wire pin4; //O1 = A1 & B1 <-- connect to io4 
wire pin5; //A2           <-- connect to io5
wire pin6; //B2           <-- connect to io6
wire pin7; //O2 = A2 | B2 <-- connect to io7
wire pin8; //GND          
assign pin4 = pin2 & pin3;
assign pin7 = pin5 | pin6;

assign pin2 = io_source_pins[1];
assign pin3 = io_source_pins[2];
assign io_capture_pins[3] = pin4;
assign pin5 = io_source_pins[4];
assign pin6 = io_source_pins[5];
assign io_capture_pins[6] = pin7;
//Virtual IC end


//Initialization
initial begin
	reset      <= 0;
	clk        <= 0;
	SCK        <= 0;
	addr       <= 0;
	request    <= 0;
	process_rqst<= 0;

	#13 reset   <= 1;  
end

always #1 clk = ~clk;

initial begin
	#23 CS 		  <= 1;

//############# setting data to pin2 (io2) ############
	#23	request <= SPI_WRREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23

	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	//zero byte
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#33 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23 addr <= 1; //io2
	#27 request <= COPY_FROM_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end


//############# setting data to pin3 (io3) ############
	#23	request <= SPI_WRREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23

	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	//zero byte
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#33 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23 addr <= 2; //io3
	#27 request <= COPY_FROM_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end


//############# setting data to (io5) ############
	#23	request <= SPI_WRREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23

	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	//zero byte
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#33 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23 addr <= 4; //io5
	#27 request <= COPY_FROM_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end

//############# setting data to (io6) ############
	#23	request <= SPI_WRREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23

	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 1; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	//zero byte
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0; //SPI transfer end
	#3 SCK <= 1;
	#3 SCK <= 0; MOSI <= 0;

	#33 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23 addr <= 5; //io6
	#27 request <= COPY_FROM_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end


//#################Start#################
	#27 request <= EXE_START; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0) begin
		#2 request <= NO_REQ;
	end

//################ READ BACK RESULTS##############

	//read pin 4
	#23 addr <= 3;
	#13 request <= COPY_TO_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end 

	#23
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#23	request <= SPI_RDREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#14
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;

//read pin 7
	#23 addr <= 6;
	#13 request <= COPY_TO_SPI; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#10 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	while(req_done == 0)begin
		#2 request <= NO_REQ;
	end 

	#23
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#23	request <= SPI_RDREQ; #4 process_rqst<= 1; #4 process_rqst<= 0;
	#23
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#14
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#3 SCK <= 1;
	#3 SCK <= 0;
	#14 request <= NO_REQ; #4 process_rqst<= 1; #4 process_rqst<= 0;


end

IO_driver fpga1(
	.clk              (clk),
	.reset            (reset),
	.MOSI             (MOSI),
	.MISO             (MISO),
	.SCK              (SCK),
	.CS_addr          (CS),
	.addr             (addr),
	.io_capture_pins  (io_capture_pins),
	.io_source_pins   (io_source_pins),
	.req_done         (req_done),
	.request          (request),
	.process_rqst     (process_rqst),
	.state_indication(state_indication)
);

endmodule