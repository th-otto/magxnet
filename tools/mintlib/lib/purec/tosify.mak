### include file for makefile
### hohmuth 13 Feb 1993
###
### tosifies name in `tosify_name'.

ifneq (,$(findstring /dev/,$(tosify_name)))
  tosify_name	:=	$(patsubst /dev/%,%,$(tosify_name))
  _devspec	:=	$(firstword $(subst /, ,$(tosify_name)))
  tosify_name	:=	$(_devspec):$(patsubst $(_devspec)%,%,$(tosify_name))
endif
tosify_name	:=	$(subst /,$(sh_backslash),$(tosify_name))

