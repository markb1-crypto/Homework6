#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VAL 128


typedef struct {
    char first[MAX_VAL];
    char second[MAX_VAL];
    char fingerprint[MAX_VAL];
    char position[MAX_VAL];
} Entry;

static int is_corruption_char(char c) {
    return (c == '#' || c == '?' || c == '!' || c == '@' || c == '&' || c == '$');
}

static char *read_and_clean_stream(FILE *fp) {
    size_t cap = 4096;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        char c = (char)ch;
        if (is_corruption_char(c)) continue;
        /* remove all whitespace so labels/values can be reconstructed across lines */
        if (isspace((unsigned char)c)) continue;

        if (len + 2 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(buf, cap);
            if (!tmp) {
                free(buf);
                printf("Memory allocation failed\n");
                return NULL;
            }
            buf = tmp;
        }
        buf[len++] = c;
    }
    buf[len] = '\0';
    return buf;
}

static void trim_inplace(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    size_t i = 0;
    while (i < n && isspace((unsigned char)s[i])) i++;
    if (i > 0) memmove(s, s + i, n - i + 1);
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static void copy_trimmed_range(char *dst, size_t dst_cap, const char *start, const char *end) {
    if (!dst || dst_cap == 0) return;
    size_t len = (end > start) ? (size_t)(end - start) : 0;
    if (len >= dst_cap) len = dst_cap - 1;
    memcpy(dst, start, len);
    dst[len] = '\0';
    trim_inplace(dst);
}

static int fp_seen(const char **seen, int seen_count, const char *fp) {
    for (int i = 0; i < seen_count; i++) {
        if (strcmp(seen[i], fp) == 0) return 1;
    }
    return 0;
}

static char *my_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static int pos_rank(const char *pos) {
    /* Output ordering requirement */
    if (strcmp(pos, "Boss") == 0) return 0;
    if (strcmp(pos, "RightHand") == 0 || strcmp(pos, "Right_Hand") == 0 || strcmp(pos, "Right Hand") == 0) return 1;
    if (strcmp(pos, "LeftHand") == 0 || strcmp(pos, "Left_Hand") == 0 || strcmp(pos, "Left Hand") == 0) return 2;
    if (strcmp(pos, "SupportRight") == 0 || strcmp(pos, "Support_Right") == 0 || strcmp(pos, "Support Right") == 0) return 3;
    if (strcmp(pos, "SupportLeft") == 0 || strcmp(pos, "Support_Left") == 0 || strcmp(pos, "Support Left") == 0) return 4;
    return 5;
}

static void normalize_position(char *pos) {
    /* After cleaning we remove whitespace, so positions become RightHand, etc. */
    if (strcmp(pos, "RightHand") == 0 || strcmp(pos, "Right_Hand") == 0) {
        strcpy(pos, "Right Hand");
    } else if (strcmp(pos, "LeftHand") == 0 || strcmp(pos, "Left_Hand") == 0) {
        strcpy(pos, "Left Hand");
    } else if (strcmp(pos, "SupportRight") == 0 || strcmp(pos, "Support_Right") == 0) {
        strcpy(pos, "Support_Right");
    } else if (strcmp(pos, "SupportLeft") == 0 || strcmp(pos, "Support_Left") == 0) {
        strcpy(pos, "Support_Left");
    }
}

static void write_entry(FILE *out, const Entry *e) {
    fprintf(out, "First Name: %s\n", e->first);
    fprintf(out, "Second Name: %s\n", e->second);
    fprintf(out, "Fingerprint: %s\n", e->fingerprint);
    fprintf(out, "Position: %s\n\n", e->position);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fclose(in);
        printf("Error opening file: %s\n", argv[2]);
        return 0;
    }

    char *stream = read_and_clean_stream(in);
    fclose(in);
    if (!stream) {
        fclose(out);
        return 0;
    }

    /* dynamic storage */
    Entry *boss = NULL;
    Entry *right_hand = NULL;
    Entry *left_hand = NULL;

    size_t sr_cap = 8, sr_len = 0;
    size_t sl_cap = 8, sl_len = 0;
    Entry *support_right = (Entry *)malloc(sr_cap * sizeof(Entry));
    Entry *support_left = (Entry *)malloc(sl_cap * sizeof(Entry));
    if (!support_right || !support_left) {
        printf("Memory allocation failed\n");
        free(stream);
        free(support_right);
        free(support_left);
        fclose(out);
        return 0;
    }

    size_t seen_cap = 32;
    int seen_len = 0;
    const char **seen = (const char **)malloc(seen_cap * sizeof(char *));
    if (!seen) {
        printf("Memory allocation failed\n");
        free(stream);
        free(support_right);
        free(support_left);
        fclose(out);
        return 0;
    }

    const char *L1 = "FirstName:";
    const char *L2 = "SecondName:";
    const char *L3 = "Fingerprint:";
    const char *L4 = "Position:";

    char *p = stream;
    while (1) {
        char *f1 = strstr(p, L1);
        if (!f1) break;
        char *f2 = strstr(f1 + strlen(L1), L2);
        char *f3 = f2 ? strstr(f2 + strlen(L2), L3) : NULL;
        char *f4 = f3 ? strstr(f3 + strlen(L3), L4) : NULL;
        if (!f2 || !f3 || !f4) break;

        Entry e;
        memset(&e, 0, sizeof(e));
        copy_trimmed_range(e.first, sizeof(e.first), f1 + strlen(L1), f2);
        copy_trimmed_range(e.second, sizeof(e.second), f2 + strlen(L2), f3);
        copy_trimmed_range(e.fingerprint, sizeof(e.fingerprint), f3 + strlen(L3), f4);

        /* Position value ends at next First Name or end of stream */
        char *next = strstr(f4 + strlen(L4), L1);
        if (!next) next = stream + strlen(stream);
        copy_trimmed_range(e.position, sizeof(e.position), f4 + strlen(L4), next);
        normalize_position(e.position);

        trim_inplace(e.fingerprint);
        if (e.fingerprint[0] != '\0' && !fp_seen(seen, seen_len, e.fingerprint)) {
            /* record seen fingerprint */
            if ((size_t)seen_len + 1 >= seen_cap) {
                seen_cap *= 2;
                const char **tmp = (const char **)realloc((void *)seen, seen_cap * sizeof(char *));
                if (!tmp) {
                    printf("Memory allocation failed\n");
                    free(stream);
                    free(support_right);
                    free(support_left);
                    for (int i = 0; i < seen_len; i++) free((void *)seen[i]);
                    free(seen);
                    fclose(out);
                    return 0;
                }
                seen = tmp;
            }
            seen[seen_len] = my_strdup(e.fingerprint);
            if (!seen[seen_len]) {
                printf("Memory allocation failed\n");
                free(stream);
                free(support_right);
                free(support_left);
                for (int i = 0; i < seen_len; i++) free((void *)seen[i]);
                free(seen);
                fclose(out);
                return 0;
            }
            seen_len++;

            int r = pos_rank(e.position);
            if (r == 0 && !boss) {
                boss = (Entry *)malloc(sizeof(Entry));
                if (!boss) { printf("Memory allocation failed\n"); break; }
                *boss = e;
            } else if (r == 1 && !right_hand) {
                right_hand = (Entry *)malloc(sizeof(Entry));
                if (!right_hand) { printf("Memory allocation failed\n"); break; }
                *right_hand = e;
            } else if (r == 2 && !left_hand) {
                left_hand = (Entry *)malloc(sizeof(Entry));
                if (!left_hand) { printf("Memory allocation failed\n"); break; }
                *left_hand = e;
            } else if (r == 3) {
                if (sr_len >= sr_cap) {
                    sr_cap *= 2;
                    Entry *tmp = (Entry *)realloc(support_right, sr_cap * sizeof(Entry));
                    if (!tmp) { printf("Memory allocation failed\n"); break; }
                    support_right = tmp;
                }
                support_right[sr_len++] = e;
            } else if (r == 4) {
                if (sl_len >= sl_cap) {
                    sl_cap *= 2;
                    Entry *tmp = (Entry *)realloc(support_left, sl_cap * sizeof(Entry));
                    if (!tmp) { printf("Memory allocation failed\n"); break; }
                    support_left = tmp;
                }
                support_left[sl_len++] = e;
            }
        }

        p = f4 + strlen(L4);
    }

    /* Output in required order */
    if (boss) write_entry(out, boss);
    if (right_hand) write_entry(out, right_hand);
    if (left_hand) write_entry(out, left_hand);
    for (size_t i = 0; i < sr_len; i++) write_entry(out, &support_right[i]);
    for (size_t i = 0; i < sl_len; i++) write_entry(out, &support_left[i]);

    fclose(out);
    free(stream);
    free(boss);
    free(right_hand);
    free(left_hand);
    free(support_right);
    free(support_left);
    for (int i = 0; i < seen_len; i++) free((void *)seen[i]);
    free(seen);
    return 0;
}
