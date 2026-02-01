#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "org_tree.h"

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

static int starts_with(const char *line, const char *prefix) {
    return strncmp(line, prefix, strlen(prefix)) == 0;
}

static void extract_value(char *dst, size_t dst_cap, const char *line, const char *prefix) {
    if (!dst || dst_cap == 0) return;
    dst[0] = '\0';
    if (!line || !prefix) return;
    const char *p = line + strlen(prefix);
    while (*p && isspace((unsigned char)*p)) p++;
    strncpy(dst, p, dst_cap - 1);
    dst[dst_cap - 1] = '\0';
    trim_inplace(dst);
}

static Node *new_node_from_fields(const char *first, const char *second, const char *fingerprint, const char *position) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) return NULL;
    strncpy(n->first, first ? first : "", MAX_FIELD - 1);
    strncpy(n->second, second ? second : "", MAX_FIELD - 1);
    strncpy(n->fingerprint, fingerprint ? fingerprint : "", MAX_FIELD - 1);
    strncpy(n->position, position ? position : "", MAX_POS - 1);
    n->left = n->right = NULL;
    n->supports_head = NULL;
    n->next = NULL;
    return n;
}

static void append_support(Node *hand, Node *support) {
    if (!hand || !support) return;
    support->next = NULL;
    if (!hand->supports_head) {
        hand->supports_head = support;
        return;
    }
    Node *cur = hand->supports_head;
    while (cur->next) cur = cur->next;
    cur->next = support;
}

static void print_node(const Node *n) {
    if (!n) return;
    printf("First Name: %s\n", n->first);
    printf("Second Name: %s\n", n->second);
    printf("Fingerprint: %s\n", n->fingerprint);
    printf("Position: %s\n\n", n->position);
}

static void free_support_list(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

Org build_org_from_clean_file(const char *path) {
    Org org;
    org.boss = NULL;
    org.left_hand = NULL;
    org.right_hand = NULL;

    FILE *fp = fopen(path, "r");
    if (!fp) {
        /* As per assignment: print error and return gracefully */
        printf("Error opening file: %s\n", path);
        return org;
    }

    char line[512];
    char first[MAX_FIELD], second[MAX_FIELD], fingerprint[MAX_FIELD], position[MAX_POS];

    while (fgets(line, sizeof(line), fp)) {
        trim_inplace(line);
        if (line[0] == '\0') continue;

        if (!starts_with(line, "First Name:")) {
            /* Unexpected line - skip until next possible entry */
            continue;
        }
        extract_value(first, sizeof(first), line, "First Name:");

        if (!fgets(line, sizeof(line), fp)) break;
        trim_inplace(line);
        extract_value(second, sizeof(second), line, "Second Name:");

        if (!fgets(line, sizeof(line), fp)) break;
        trim_inplace(line);
        extract_value(fingerprint, sizeof(fingerprint), line, "Fingerprint:");

        if (!fgets(line, sizeof(line), fp)) break;
        trim_inplace(line);
        extract_value(position, sizeof(position), line, "Position:");

        Node *node = new_node_from_fields(first, second, fingerprint, position);
        if (!node) {
            printf("Memory allocation failed\n");
            fclose(fp);
            free_org(&org);
            return org;
        }

        if (strcmp(position, "Boss") == 0) {
            org.boss = node;
        } else if (strcmp(position, "Left Hand") == 0 || strcmp(position, "Left_Hand") == 0) {
            org.left_hand = node;
        } else if (strcmp(position, "Right Hand") == 0 || strcmp(position, "Right_Hand") == 0) {
            org.right_hand = node;
        } else if (strcmp(position, "Support_Left") == 0 || strcmp(position, "Support Left") == 0) {
            append_support(org.left_hand, node);
        } else if (strcmp(position, "Support_Right") == 0 || strcmp(position, "Support Right") == 0) {
            append_support(org.right_hand, node);
        } else {
            /* Unknown position: free and ignore */
            free(node);
        }

        /*optional blank line between entries */
        long pos = ftell(fp);
        if (fgets(line, sizeof(line), fp)) {
            trim_inplace(line);
            if (line[0] != '\0') {
                fseek(fp, pos, SEEK_SET);
            }
        }
    }

    fclose(fp);

    /* Connect tree pointers */
    if (org.boss) {
        org.boss->left = org.left_hand;
        org.boss->right = org.right_hand;
    }

    return org;
}

void print_tree_order(const Org *org) {
    if (!org || !org->boss) return;

    print_node(org->boss);

    if (org->left_hand) {
        print_node(org->left_hand);
        Node *s = org->left_hand->supports_head;
        while (s) {
            print_node(s);
            s = s->next;
        }
    }

    if (org->right_hand) {
        print_node(org->right_hand);
        Node *s = org->right_hand->supports_head;
        while (s) {
            print_node(s);
            s = s->next;
        }
    }
}

void free_org(Org *org) {
    if (!org) return;

    if (org->left_hand) {
        free_support_list(org->left_hand->supports_head);
        org->left_hand->supports_head = NULL;
    }
    if (org->right_hand) {
        free_support_list(org->right_hand->supports_head);
        org->right_hand->supports_head = NULL;
    }

    free(org->boss);
    free(org->left_hand);
    free(org->right_hand);

    org->boss = NULL;
    org->left_hand = NULL;
    org->right_hand = NULL;
}
