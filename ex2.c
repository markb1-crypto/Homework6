#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "org_tree.h"

#define FP_LEN 9

static void print_success(int mask, char *op, char* fingerprint, char* First_Name, char* Second_Name)
{
    printf("Successful Decrypt! The Mask used was mask_%d of type (%s) and The fingerprint was %.*s belonging to %s %s\n",
           mask, op, FP_LEN, fingerprint, First_Name, Second_Name);
}

static void print_unsuccess()
{
    printf("Unsuccesful decrypt, Looks like he got away\n");
}

static int read_cipher_bits(const char *path, uint8_t out_bytes[FP_LEN]) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("Error opening file: %s\n", path);
        return 0;
    }

    char line[128];
    int count = 0;
    while (count < FP_LEN && fgets(line, sizeof(line), fp)) {
        /* trim */
        size_t n = strcspn(line, "\r\n");
        line[n] = '\0';
        if (line[0] == '\0') continue;
        if (strlen(line) < 8) continue;

        uint8_t v = 0;
        for (int i = 0; i < 8; i++) {
            v <<= 1;
            if (line[i] == '1') v |= 1;
            else if (line[i] != '0') {
                /* invalid char - treat as failure */
                fclose(fp);
                return 0;
            }
        }
        out_bytes[count++] = v;
    }

    fclose(fp);
    return count == FP_LEN;
}

static int fingerprint_matches(const char fp[FP_LEN + 1], const uint8_t cipher[FP_LEN], int mask, int use_xor) {
    for (int i = 0; i < FP_LEN; i++) {
        uint8_t plain = (uint8_t)fp[i];
        uint8_t enc = use_xor ? (uint8_t)(plain ^ (uint8_t)mask) : (uint8_t)(plain & (uint8_t)mask);
        if (enc != cipher[i]) return 0;
    }
    return 1;
}

static Node *find_match_in_org(const Org *org, const uint8_t cipher[FP_LEN], int mask, int use_xor) {
    if (!org || !org->boss) return NULL;

    /* check boss */
    if (fingerprint_matches(org->boss->fingerprint, cipher, mask, use_xor)) return org->boss;

    /* check left hand + supports */
    if (org->left_hand && fingerprint_matches(org->left_hand->fingerprint, cipher, mask, use_xor)) return org->left_hand;
    if (org->left_hand) {
        Node *s = org->left_hand->supports_head;
        while (s) {
            if (fingerprint_matches(s->fingerprint, cipher, mask, use_xor)) return s;
            s = s->next;
        }
    }

    /* check right hand + supports */
    if (org->right_hand && fingerprint_matches(org->right_hand->fingerprint, cipher, mask, use_xor)) return org->right_hand;
    if (org->right_hand) {
        Node *s = org->right_hand->supports_head;
        while (s) {
            if (fingerprint_matches(s->fingerprint, cipher, mask, use_xor)) return s;
            s = s->next;
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }

    uint8_t cipher[FP_LEN];
    if (!read_cipher_bits(argv[2], cipher)) {
        /* Error already printed if file couldn't open; otherwise just exit gracefully */
        return 0;
    }

    int s = atoi(argv[3]);

    Org org = build_org_from_clean_file(argv[1]);
    if (!org.boss) {
        /* build_org_from_clean_file prints file error if any */
        free_org(&org);
        return 0;
    }

    for (int mask = s; mask <= s + 10; mask++) {
        Node *n_xor = find_match_in_org(&org, cipher, mask, 1);
        if (n_xor) {
            print_success(mask, "XOR", n_xor->fingerprint, n_xor->first, n_xor->second);
            free_org(&org);
            return 0;
        }

        Node *n_and = find_match_in_org(&org, cipher, mask, 0);
        if (n_and) {
            print_success(mask, "AND", n_and->fingerprint, n_and->first, n_and->second);
            free_org(&org);
            return 0;
        }
    }

    print_unsuccess();
    free_org(&org);
    return 0;
}
