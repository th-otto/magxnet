/*
	execve for MiNT/TOS; written by Eric R. Smith, and
	placed in the public domain
*/

#include <process.h>
#include <unistd.h>

int execve(path, argv, envp)
const char *path;
char *const *argv;
char *const *envp;
{
	return _spawnve(P_OVERLAY, path, argv, envp);
}
