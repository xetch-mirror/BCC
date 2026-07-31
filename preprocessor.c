void preprocess(const char* source_code, char* output_buffer) {
    char line[256];
    const char* src = source_code;
    int skip_depth = 0;

    while (*src) {
        int len = 0;
        while (*src && *src != '\n' && len < 255) {
            line[len++] = *src++;
        }
        if (*src == '\n') src++;
        line[len] = '\0';

        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        // Directive Parsing
        if (strncmp(p, "#ifdef", 6) == 0) {
            char name[MAX_NAME];
            sscanf(p + 6, "%s", name);
            if (skip_depth > 0 || !is_defined(name)) skip_depth++;
            continue;
        } 
        else if (strncmp(p, "#ifndef", 7) == 0) {
            char name[MAX_NAME];
            sscanf(p + 7, "%s", name);
            if (skip_depth > 0 || is_defined(name)) skip_depth++;
            continue;
        } 
        else if (strncmp(p, "#else", 5) == 0) {
            if (skip_depth == 1) skip_depth = 0;
            else if (skip_depth == 0) skip_depth = 1;
            continue;
        } 
        else if (strncmp(p, "#endif", 6) == 0) {
            if (skip_depth > 0) skip_depth--;
            continue;
        }

        if (skip_depth > 0) continue;

        if (strncmp(p, "#define", 7) == 0) {
            char name[MAX_NAME], val[MAX_VAL] = "";
            int parsed = sscanf(p + 7, "%s %s", name, val);
            if (parsed >= 1) {
                add_macro(name, (parsed == 2) ? val : "1");
            }
            continue;
        }

        if (strncmp(p, "#include", 8) == 0) {
            char header_path[128];
            if (sscanf(p + 8, " \"%[^\"]\"", header_path) == 1) {
                char header_src[4096] = {0};
                if (vfs_read_file(header_path, header_src, sizeof(header_src))) {
                    preprocess(header_src, output_buffer);
                }
            }
            continue;
        }

        // Macro expansion with word boundary check (bounds-checked)
        for (int i = 0; i < macro_count; i++) {
            char temp[512];
            char* curr = p;
            char* out = temp;
            char* out_end = temp + sizeof(temp) - 1; // reserve room for '\0'
            int mlen = strlen(macro_table[i].name);
            int vlen = strlen(macro_table[i].value);
            int overflowed = 0;

            while (*curr) {
                char* match = strstr(curr, macro_table[i].name);
                if (match) {
                    // Check word boundaries
                    char prev = (match == p) ? ' ' : *(match - 1);
                    char next = *(match + mlen);

                    if (!isalnum(prev) && prev != '_' && !isalnum(next) && next != '_') {
                        int pre_len = (int)(match - curr);

                        // Bounds check: prefix segment + replacement value
                        if (out + pre_len + vlen > out_end) {
                            overflowed = 1;
                            break;
                        }

                        // Copy up to match
                        memcpy(out, curr, pre_len);
                        out += pre_len;
                        // Copy replacement
                        memcpy(out, macro_table[i].value, vlen);
                        out += vlen;
                        curr = match + mlen;
                        continue;
                    }
                }
                if (out >= out_end) {
                    overflowed = 1;
                    break;
                }
                *out++ = *curr++;
            }

            if (overflowed) {
                fprintf(stderr,
                    "[preprocessor] warning: macro expansion for '%s' exceeds %zu bytes, "
                    "line left unexpanded for this pass\n",
                    macro_table[i].name, sizeof(temp));
                continue; // skip this macro's substitution on this line; try the next macro
            }

            *out = '\0';

            // Bounds check before writing back into the 256-byte line buffer
            if ((int)strlen(temp) > 255) {
                fprintf(stderr,
                    "[preprocessor] warning: expanded line exceeds 255 bytes, truncating\n");
                temp[255] = '\0';
            }
            strcpy(p, temp);
        }

        strcat(output_buffer, p);
        strcat(output_buffer, "\n");
    }
}
