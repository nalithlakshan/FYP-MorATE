// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on
module  tb;

reg	  aclr;
reg	[0:0]  data;
reg	  rdclk;
reg	  rdreq;
reg	  wrclk;
reg	  wrreq;
wire[0:0]  q;
wire  rdempty;

//Initialization
initial begin
	aclr   <= 1;
	wrclk  <= 0;
	rdclk  <= 0;
	wrreq  <= 0;
	rdreq  <= 0;

	#13 aclr   <= 0;  
end

always #1 rdclk = ~rdclk;

initial begin
	#23	wrreq <= 1;
	#23
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0; //SPI transfer end
	#33 wrreq <= 0;

	#13 rdreq<= 1;
	while(rdempty == 0)begin
		#1 rdreq<= 1;
	end
	#3 rdreq <=0;
	#43

	#23	wrreq <= 1;
	#23
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 1;
	#3 wrclk <= 1;
	#3 wrclk <= 0; data <= 0; //SPI transfer end
	#23	wrreq <= 0;

	#13 rdreq<= 1;
	while(rdempty == 0)begin
		#1 rdreq<= 1;
	end
	#3 rdreq <=0;
end

fifo_2clk fifo2c(
	.aclr   (aclr),
	.data   (data),
	.rdclk  (rdclk),
	.rdreq  (rdreq),
	.wrclk  (wrclk),
	.wrreq  (wrreq),
	.q      (q),
	.rdempty(rdempty)
);

endmodule