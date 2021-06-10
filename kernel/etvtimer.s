	.globl new_etv_timer
	.globl old_etv_timer

	.globl x114ce

	.text
	
	.dc.l 0x58425241 /* 'XBRA' */
	.dc.l 0x53434b4d /* 'SCKM' */
old_etv_timer:
	.dc.l 0
new_etv_timer:

x114ce:
