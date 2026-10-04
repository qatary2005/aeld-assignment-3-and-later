#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    if (cmd == NULL) {
        return false;
    }

    int ret = system(cmd);
    if (ret == -1) {
        return false;
    }

    return (WIFEXITED(ret) && (WEXITSTATUS(ret) == 0));
}

/**
 * Helper function to execute command with optional stdout redirect
 * @param command - the command array argv
 * @param redirect_file_may_be_null - when not null, the filename to use as output redirect.
 * @return true on success, false on error
 */
static bool do_exec_redirect_to_file(char **command, const char *redirect_file_may_be_null)
{
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return false;
    }

    if (pid == 0) {
        // Child process
        if (redirect_file_may_be_null != NULL) {
            int fd = open(redirect_file_may_be_null, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        execv(command[0], command);
        // If execv returns, an error occurred
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return false;
        }

        if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
            return true;
        }

        return false;
    }
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/
bool do_exec(int count, ...)
{
    if (count <= 0) {
        return false;
    }

    va_list args;
    va_start(args, count);
    char * command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    return do_exec_redirect_to_file(command, NULL);
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* for documentation of all other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    if (count <= 0 || outputfile == NULL) {
        return false;
    }

    va_list args;
    va_start(args, count);
    char * command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    return do_exec_redirect_to_file(command, outputfile);
}
