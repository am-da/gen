#include "mppl_compiler.h"

FILE *input, *output;
int main(int nc, char *np[]) {

    int is_success = EXIT_FAILURE;
    if(nc < 2) {
        error("File name is not given.");
        return is_success;
    }

    if(init_scan(np[1], &input) == ERROR) return is_success;
    
    if(init_outfile(np[1], &output) == ERROR) return is_success;

    is_success = Parse_program();

    end_scan(input);
    end_outfile(output);

    return is_success;
}

int error(const char *mes, ...) {
    va_list args;
    char out[MAXSTRSIZE];
    va_start(args, mes);
    vsnprintf(out, MAXSTRSIZE, mes, args);
    va_end(args);

    fflush(stdout);
    fprintf(stderr, "\n ERROR: line %d %s\n", get_linenum(), out);
    return ERROR;
}
