//3. Write a C application “writer” (finder-app/writer.c)  which can be used as an alternative to the “writer.sh” test script created in assignment1 and using File IO as described in LSP chapter 2.  See the Assignment 1 requirements for the writer.sh test script and these additional instructions:
//One difference from the write.sh instructions in Assignment 1:  You do not need to make your "writer" utility create directories which do not exist.  You can assume the directory is created by the caller.
//Setup syslog logging for your utility using the LOG_USER facility.
//Use the syslog capability to write a message “Writing <string> to <file>” where <string> is the text string written to file (second argument) and <file> is the file created by the script.  This should be written with LOG_DEBUG level.
//Use the syslog capability to log any unexpected errors with LOG_ERR level.


#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

void write_to_file(const char* file_path, const char* text) {
    FILE* file = fopen(file_path, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Failed to open file: %s", file_path);
        return;
    }

    fprintf(file, "%s", text);
    fclose(file);

    syslog(LOG_DEBUG, "Writing \"%s\" to %s", text, file_path);
}

int main(int argc, char* argv[]) {
    openlog("writer", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);

    if (argc < 3) {
        syslog(LOG_ERR, "Insufficient arguments. Usage: writer <file> <text>");
        closelog();
        return EXIT_FAILURE;
    }

    const char* file_path = argv[1];
    const char* text = argv[2];

    write_to_file(file_path, text);

    closelog();
    return EXIT_SUCCESS;
}

