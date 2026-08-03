// bcc_flags.h — global flag state
int flag_wall = 0;
int flag_wextra = 0;
int flag_freestanding = 0;
int flag_nostdlib = 0;

char *input_file = 0;

void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-Wall") == 0) {
            flag_wall = 1;
        } else if (strcmp(argv[i], "-Wextra") == 0) {
            flag_wextra = 1;
            flag_wall = 1; // -Wextra implies -Wall, same as real gcc
        } else if (strcmp(argv[i], "-ffreestanding") == 0) {
            flag_freestanding = 1;
        } else if (strcmp(argv[i], "-nostdlib") == 0) {
            flag_nostdlib = 1;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "BCC: unknown flag '%s'\n", argv[i]);
        }
    }
}