BEGIN {
	output = 0
	retcode = 1
	printf "\t.INCLUDE \'osmacros.s\'\n\n\t.EXPORT %s\n\n", name
}

/\.ENDMOD/ && output {
	retcode = 0
	exit
}

output

!output && $2 == name ":" {
	output = 1
	print $2
}

END {
	exit retcode
}
