/* struct socket */		
		.OFFSET 0
so_type:	.ds.w 1
so_state:	.ds.w 1
so_flags:	.ds.w 1
so_conn:	.ds.l 1
so_iconn_q:	.ds.l 1
so_next:	.ds.l 1
so_ops:		.ds.l 1
so_data:	.ds.l 1
so_error:	.ds.w 1
so_pgrp:	.ds.w 1
so_rsel:	.ds.l 1
so_wsel:	.ds.l 1
so_xsel:	.ds.l 1
so_date:	.ds.l 1
so_time:	.ds.w 1
so_lockpid:	.ds.w 1

/* struct dom_ops */
		.OFFSET 0
dom_domain:	.ds.w 1
dom_next:	.ds.l 1
dom_attach:	.ds.l 1
dom_dup:	.ds.l 1
dom_abort:	.ds.l 1
dom_detach:	.ds.l 1
dom_bind:	.ds.l 1
dom_connect: .ds.l 1
dom_socketpair: .ds.l 1
dom_accept:	.ds.l 1
dom_getname: .ds.l 1
dom_select:	.ds.l 1
dom_ioctl:	.ds.l 1
dom_listen:	.ds.l 1
dom_send:	.ds.l 1
dom_recv:	.ds.l 1
dom_shutdown: .ds.l 1
dom_setsockopt: .ds.l 1
dom_getsockopt: .ds.l 1
