	.IFEQ GNUC
MSHORT = 1
	.ENDC

     .OFFSET 0

ddev_open:     DS.L      1
ddev_close:    DS.L      1
ddev_read:     DS.L      1
ddev_write:    DS.L      1
ddev_stat:     DS.L      1
ddev_seek:     DS.L      1
ddev_datime:   DS.L      1
ddev_ioctl:    DS.L      1
ddev_delete:   DS.L      1
ddev_getc:     DS.L      1
ddev_getline:  DS.L      1
ddev_putc:     DS.L      1

     .OFFSET 0

fd_dmd:        DS.L      1    /* 0x00: Pointer to DMD                      */
fd_refcnt:     DS.W      1    /* 0x04: Ref. counter for closing or -1      */
fd_mode:       DS.W      1    /* 0x06: Opening mode (0,1,2) and flags      */
fd_dev:        DS.L      1    /* 0x08: Pointer to MX_DEV                   */
fd_ddev:       DS.L      1    /* 0x0c: Pointer to MX_DDEV                  */
fd_name:       DS.B      11   /* 0x10: DD_FD: Name in internal format      */
fd_attr:       DS.B      1    /* 0x1b: Attribute                           */
fd_owner:      DS.L      1    /* 0x1c: DD_FD: Owner of an FD or NULL       */
fd_parent:     DS.L      1    /* 0x20: Pointer to the parent-DD_FD         */
fd_children:   DS.L      1    /* 0x24: DD_FD: List of all children         */
fd_next:       DS.L      1    /* 0x28: Pointer to FDs in same directory    */
fd_multi:      DS.L      1    /* 0x2c: Pointer to FD of the same file      */
fd_multi1:     DS.L      1    /* 0x30: Pointer to first list element       */
fd_fpos:       DS.L      1    /* 0x34: Position of file pointer            */
fd_dirch:      DS.B      1    /* 0x38: Bit0: "dirty"                       */
fd_unused:     DS.B      1    /* 0x39: */
fd_time:       DS.W      1    /* 0x3a: Time  (8086)                        */
fd_date:       DS.W      1    /* 0x3c: Date  (8086)                        */
fd_stcl:       DS.W      1    /* 0x3e: Start cluster                       */
fd_len:        DS.L      1    /* 0x40: File length in bytes                */
fd_dirpos:     DS.L      1    /* 0x44: Pos.of associated entry in directory*/
fd_user1:      DS.L      1    /* 0x48: */
fd_user2:      DS.L      1    /* 0x4c: */

     .OFFSET 0

dta_sname:     DS.B      12   /* 0x00: Search name (from Fsfirst)          */
dta_usr1:      DS.L      1    /* 0x0c: Available for free use              */
dta_usr2:      DS.L      1    /* 0x10: Available for free use              */
dta_drive:     DS.B      1    /* 0x14: Logical drive (0..25)               */
dta_attr:      DS.B      1    /* 0x15: Found attribute                     */
dta_time:      DS.W      1    /* 0x16: Found time                          */
dta_date:      DS.W      1    /* 0x18: Found date                          */
dta_len:       DS.L      1    /* 0x1a: Found length                        */
dta_name:      DS.B      14   /* 0x1e: Found filename                      */

     .OFFSET 0

/* The first three entries exist for all file systems */
d_xfs:         DS.L      1    /* 0x00: The file system driver              */
d_drive:       DS.W      1    /* 0x04: Drive number 0..25                  */
d_root:        DS.L      1    /* 0x06: Pointer to DD of the root           */
d_biosdev:     DS.W      1    /* 0x0a: BIOS drive or -1                    */
d_driver:      DS.L      1    /* 0x0c: With devcode this defines the medium*/
d_devcode:     DS.L      1    /* 0x10: e.g. SCSI target & drive            */
/* This entry exists only for DOS file systems         */
d_dfs:         DS.L      1    /* 0x14: DOS-specific file system driver     */

     .OFFSET 0

dir_name:      DS.B     11    /* 0x00: File name                           */
dir_attr:      DS.B      1    /* 0x0b: Attribute                           */
dir_usr1:      DS.W      1    /* 0x0c: Available for free use              */
dir_usr2:      DS.L      1    /* 0x0e: Available for free use              */
dir_usr3:      DS.L      1    /* 0x12: Available for free use              */
dir_time:      DS.W      1    /* 0x16: Time of last modification           */
dir_date:      DS.W      1    /* 0x18: Date of last modification           */
dir_stcl:      DS.W      1    /* 0x1a: First cluster                       */
dir_flen:      DS.L      1    /* 0x1c: File length                         */

     .OFFSET 0

dfs_name:      DS.B      8
dfs_next:      DS.L      1
dfs_init:      DS.L      1
dfs_sync:      DS.L      1
dfs_drv_open:  DS.L      1
dfs_drv_close: DS.L      1
dfs_dfree:     DS.L      1
dfs_sfirst:    DS.L      1
dfs_snext:     DS.L      1
dfs_ext_fd:    DS.L      1
dfs_fcreate:   DS.L      1
dfs_fxattr:    DS.L      1
dfs_dir2index: DS.L      1
dfs_readlink:  DS.L      1
dfs_dir2FD:    DS.L      1
dfs_fdelete:   DS.L      1
dfs_pathconf:  DS.L      1

/* Supported Dcntl modes */
DFS_GETINFO    =  0x1100     /* XFS_DOS: give pointer to DFS-Kernel */
DFS_INSTDFS    =  0x1200     /* XFS_DOS: install a DFS */
DEV_M_INSTALL  =  0xcd00     /* DFS_U */

