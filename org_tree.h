#ifndef ORG_TREE_H
#define ORG_TREE_H
#define MAX_FIELD 128
#define MAX_POS   32

typedef struct Node Node;

struct Node {
    char first[MAX_FIELD];
    char second[MAX_FIELD];
    char fingerprint[MAX_FIELD];
    char position[MAX_POS];

    // Tree pointers (used for Boss / Hands)
    Node *left;   // Boss->Left Hand
    Node *right;  // Boss->Right Hand

    // Support list head (used for Hands)
    Node *supports_head;

    // Next pointer (used only when node is in a supports linked list)
    Node *next;
};

typedef struct {
    Node *boss;
    Node *left_hand;
    Node *right_hand;
} Org;

Org build_org_from_clean_file(const char *path);
void print_tree_order(const Org *org);
void free_org(Org *org);

#endif // ORG_TREE_H