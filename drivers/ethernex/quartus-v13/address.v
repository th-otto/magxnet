// At the time of this writing this module is unused.
// It was planned to replace the NAND7, NOR3 and NOT on the address lines in "falcon.bdf"

module address (A,RESET,CLK,AS,CE);

	input [23:16] A;
	input RESET,CLK,AS;
	output CE;
	reg CE;

	always @( posedge CLK or negedge RESET) begin
		if (!RESET) begin
			CE <= 1'b1;
		end else begin
			if (A == 8'hF1 & !AS) begin
				CE <= 1'b0;
			end else begin
				CE <= 1'b1;
			end
		end
	end
		
endmodule
