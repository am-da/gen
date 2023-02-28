#include "mppl_compiler.h"

FILE *input, *output;
int main(int nc, char *np[]) {

    int sf = EXIT_FAILURE;
    if(nc < 2) {
        error("File name is not found.");
        return sf;
    }

    if(init_scan(np[1], &input) == ERROR) return sf;
    if(init_outfile(np[1], &output) == ERROR) return sf;

    sf = Parse_program();
    end_scan(input);
    end_outfile(output);

    return sf;
}

int error(const char *mes, ...) {
    va_list num;
    char result[MAXSTRSIZE];
    va_start(num, mes);
    vsnprintf(result, MAXSTRSIZE, mes, num);
    va_end(num);

    fflush(stdout);
    fprintf(stderr, "ERROR line %d %s\n", get_linenum(), result);
    return ERROR;
}
