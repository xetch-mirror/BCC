#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "loader.h"
#inclide "preprocessor.h"

#define MAX_NAME     64
#define MAX_VAL      192
#define MAX_MACROS   256

typedef struct { char name[MAX_NAME]; char value[MAX_VAL]; } macro_t;

static macro_t macro_table[MAX_MACROS];
static int macro_count = 0;

static int is_defined(const char *name)
{
  int i;
  for (i = 0; i < macro_count; i++)
    if (strcmp(macro_table[i].name, name) == 0) return 1;
  return 0;
}

static void add_macro(const char *name, const char *value)
{
  int i;
  for (i = 0; i < macro_count; i++) {
    if (strcmp(macro_table[i].name, name) == 0) {
      strncpy(macro_table[i].value, value, MAX_VAL - 1);
      macro_table[i].value[MAX_VAL - 1] = 0;
      return;
    }
  }
  if (macro_count >= MAX_MACROS) {
    fprintf(stderr, "[preprocessor] warning: macro table full, ignoring #define %s\n", name);
    return;
  }
  strncpy(macro_table[macro_count].name, name, MAX_NAME - 1);
  macro_table[macro_count].name[MAX_NAME - 1] = 0;
  strncpy(macro_table[macro_count].value, value, MAX_VAL - 1);
  macro_table[macro_count].value[MAX_VAL - 1] = 0;
  macro_count++;
}

void preprocess(const char *source_code, char *output_buffer, size_t output_buffer_size)
{
    char line[256];
    const char *src = source_code;
    int skip_depth = 0;

    output_buffer[0] = 0;

    while (*src) {
        int len = 0;
        while (*src && *src != '\n' && len < 255) {
            line[len++] = *src++;
        }
        if (*src == '\n') src++;
        line[len] = '\0';

        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (strncmp(p, "#ifdef", 6) == 0) {
            char name[MAX_NAME];
            sscanf(p + 6, "%63s", name);
            if (skip_depth > 0 || !is_defined(name)) skip_depth++;
            continue;
        }
        else if (strncmp(p, "#ifndef", 7) == 0) {
            char name[MAX_NAME];
            sscanf(p + 7, "%63s", name);
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
            int parsed = sscanf(p + 7, "%63s %191s", name, val);
            if (parsed >= 1) add_macro(name, (parsed == 2) ? val : "1");
            continue;
        }

        if (strncmp(p, "#include", 8) == 0) {
            char header_path[128];
            if (sscanf(p + 8, " \"%127[^\"]\"", header_path) == 1) {
                char *header_src = load_source(header_path);
                if (header_src) {
                    size_t used = strlen(output_buffer);
                    preprocess(header_src, output_buffer + used, output_buffer_size - used);
                    free(header_src);
                } else {
                    fprintf(stderr, "[preprocessor] warning: could not open included file %s\n", header_path);
                }
            }
            continue;
        }

        for (int i = 0; i < macro_count; i++) {
            char temp[512];
            char *curr = p;
            char *out = temp;
            char *out_end = temp + sizeof(temp) - 1;
            int mlen = strlen(macro_table[i].name);
            int vlen = strlen(macro_table[i].value);
            int overflowed = 0;

            while (*curr) {
                char *match = strstr(curr, macro_table[i].name);
                if (match) {
                    char prev = (match == p) ? ' ' : *(match - 1);
                    char next = *(match + mlen);

                    if (!isalnum((unsigned char)prev) && prev != '_' &&
                        !isalnum((unsigned char)next) && next != '_') {
                        int pre_len = (int)(match - curr);
                        if (out + pre_len + vlen > out_end) { overflowed = 1; break; }
                        memcpy(out, curr, pre_len);
                        out += pre_len;
                        memcpy(out, macro_table[i].value, vlen);
                        out += vlen;
                        curr = match + mlen;
                        continue;
                    }
                }
                if (out >= out_end) { overflowed = 1; break; }
                *out++ = *curr++;
            }

            if (overflowed) {
                fprintf(stderr,
                    "[preprocessor] warning: macro expansion for '%s' exceeds %zu bytes, "
                    "line left unexpanded for this pass\n",
                    macro_table[i].name, sizeof(temp));
                continue;
            }

            *out = '\0';
            if ((int)strlen(temp) > 255) {
                fprintf(stderr, "[preprocessor] warning: expanded line exceeds 255 bytes, truncating\n");
                temp[255] = '\0';
            }
            strcpy(p, temp);
        }

        size_t used = strlen(output_buffer);
        size_t need = strlen(p) + 1;
        if (used + need + 1 > output_buffer_size) {
            fprintf(stderr, "[preprocessor] warning: output buffer full, truncating preprocessed source\n");
            return;
        }
        strcat(output_buffer, p);
        strcat(output_buffer, "\n");
    }
}