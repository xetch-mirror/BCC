void preprocess(const char* source_code, char* output_buffer) {
    char line[256];
    const char* src = source_code;
    int skipping = 0; // set to 1

    while (*src) {
        // buffer
        int len = 0;
        while (*src && *src != '\n' && len < 255) {
            line[len++] = *src++;
        }
        if (*src == '\n') src++;
        line[len] = '\0';

        // whitespace
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        // do ifndef
        if (strncmp(p, "#ifdef", 6) == 0) {
            char name[MAX_NAME];
            sscanf(p + 6, "%s", name);
            skipping = !is_defined(name);
            continue; // Do not output directive line to compiler
        } 
        else if (strncmp(p, "#ifndef", 7) == 0) {
            char name[MAX_NAME];
            sscanf(p + 7, "%s", name);
            skipping = is_defined(name);
            continue;
        } 
        else if (strncmp(p, "#else", 5) == 0) {
            skipping = !skipping;
            continue;
        } 
        else if (strncmp(p, "#endif", 6) == 0) {
            skipping = 0;
            continue;
        }

        // skip bad ifdef
        if (skipping) continue;

        // define
        if (strncmp(p, "#define", 7) == 0) {
            char name[MAX_NAME], val[MAX_VAL] = "";
            int parsed = sscanf(p + 7, "%s %s", name, val);
            if (parsed >= 1) {
                // if no value provided
                add_macro(name, (parsed == 2) ? val : "1");
            }
            continue; // input directive
        }

        // do header files
        if (strncmp(p, "#include", 8) == 0) {
            char header_path[128];
            if (sscanf(p + 8, " \"%[^\"]\"", header_path) == 1) {
                // read header file using OS Virtual File System
                char header_src[4096] = {0};
                if (vfs_read_file(header_path, header_src, sizeof(header_src))) {
                    // recursively preprocess the header text right here
                    preprocess(header_src, output_buffer + strlen(output_buffer));
                }
            }
            continue;
        }

        // 5. expand defined macros in regular C code lines
        for (int i = 0; i < macro_count; i++) {
            char* match;
            while ((match = strstr(p, macro_table[i].name)) != NULL) {
                // swap macro token with its stored value
                char temp[256];
                int prefix_len = match - p;
                strncpy(temp, p, prefix_len);
                temp[prefix_len] = '\0';
                
                strcat(temp, macro_table[i].value);
                strcat(temp, match + strlen(macro_table[i].name));
                
                strcpy(p, temp); // line is now updated with replaced string
            }
        }

        // append processed line to output buffer
        strcat(output_buffer, p);
        strcat(output_buffer, "\n");
    }
}
