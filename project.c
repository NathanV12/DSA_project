/*
 * =====================================================================
 *            PUBLIC TRANSPORT LOST ITEM RECOVERY SYSTEM
 *            KTU 2024 Scheme - Data Structures and Algorithms
 * =====================================================================
 *
 *  INDEX
 *  -----
 *  Section 0 : Header files, constants and input helper
 *  Section 1 : MODULE 1 - Circular Queue      (pending claims, FIFO)
 *  Section 2 : MODULE 2 - Singly Linked List  (master list of items)
 *  Section 3 : MODULE 3 - Binary Search Tree  (fast search by item ID)
 *  Section 4 : MODULE 4 - Hash Table          (claimant details)
 *  Section 5 : System operations              (menu actions)
 *  Section 6 : Main function                  (menu driver)
 *
 * =====================================================================
 */


/* =====================================================================
 * SECTION 0 : HEADER FILES, CONSTANTS AND INPUT HELPER
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN        50
#define MAX_QUEUE  50
#define HASH_SIZE  53

/* Reads a positive integer; asks again if input is invalid */
int readInt(const char *msg)
{
    int x, c;

    printf("%s", msg);

    while (scanf("%d", &x) != 1 || x <= 0)
    {
        while ((c = getchar()) != '\n' && c != EOF);

        if (c == EOF)
        {
            exit(0);
        }

        printf("Enter a positive number: ");
    }

    return x;
}


/* =====================================================================
 * SECTION 1 : MODULE 1 - CIRCULAR QUEUE
 * Stores claim IDs so that claims are processed first-come, first-served
 * ===================================================================== */

int queue[MAX_QUEUE];
int front = 0;
int rear  = -1;
int count = 0;

/* 1.1 Add a claim ID at the rear */
void enqueue(int x)
{
    rear = (rear + 1) % MAX_QUEUE;
    queue[rear] = x;
    count++;
}

/* 1.2 Remove and return the claim ID at the front */
int dequeue()
{
    int x = queue[front];

    front = (front + 1) % MAX_QUEUE;
    count--;

    return x;
}


/* =====================================================================
 * SECTION 2 : MODULE 2 - SINGLY LINKED LIST
 * Master list of all unclaimed items
 * ===================================================================== */

struct Item
{
    int id;
    char name[LEN];
    struct Item *next;
};

struct Item *head = NULL;

/* 2.1 Insert a new item at the beginning of the list */
struct Item *insertItem(int id, char name[])
{
    struct Item *n = malloc(sizeof(struct Item));

    n->id = id;
    strcpy(n->name, name);
    n->next = head;
    head = n;

    return n;
}

/* 2.2 Delete an item from the list by ID */
void deleteItem(int id)
{
    struct Item *cur  = head;
    struct Item *prev = NULL;

    while (cur != NULL && cur->id != id)
    {
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL)
    {
        return;
    }

    if (prev == NULL)
    {
        head = cur->next;
    }
    else
    {
        prev->next = cur->next;
    }

    free(cur);
}

/* 2.3 Display all items in the list */
void displayItems()
{
    struct Item *p;

    if (head == NULL)
    {
        printf("\nNo unclaimed items.\n");
        return;
    }

    printf("\nID\tItem\n");

    for (p = head; p != NULL; p = p->next)
    {
        printf("%d\t%s\n", p->id, p->name);
    }
}


/* =====================================================================
 * SECTION 3 : MODULE 3 - BINARY SEARCH TREE
 * Index on item ID; each node points to the item in the linked list
 * ===================================================================== */

struct TreeNode
{
    int id;
    struct Item *item;
    struct TreeNode *left;
    struct TreeNode *right;
};

struct TreeNode *root = NULL;

/* 3.1 Insert a node into the BST */
struct TreeNode *insertBST(struct TreeNode *t, int id, struct Item *item)
{
    if (t == NULL)
    {
        t = malloc(sizeof(struct TreeNode));
        t->id    = id;
        t->item  = item;
        t->left  = NULL;
        t->right = NULL;
    }
    else if (id < t->id)
    {
        t->left = insertBST(t->left, id, item);
    }
    else if (id > t->id)
    {
        t->right = insertBST(t->right, id, item);
    }

    return t;
}

/* 3.2 Search for an item by ID */
struct Item *searchBST(struct TreeNode *t, int id)
{
    while (t != NULL && t->id != id)
    {
        if (id < t->id)
        {
            t = t->left;
        }
        else
        {
            t = t->right;
        }
    }

    if (t == NULL)
    {
        return NULL;
    }

    return t->item;
}

/* 3.3 Delete a node from the BST */
struct TreeNode *deleteBST(struct TreeNode *t, int id)
{
    struct TreeNode *temp;

    if (t == NULL)
    {
        return NULL;
    }

    if (id < t->id)
    {
        t->left = deleteBST(t->left, id);
    }
    else if (id > t->id)
    {
        t->right = deleteBST(t->right, id);
    }
    else
    {
        /* Case 1 and 2 : node has zero or one child */
        if (t->left == NULL)
        {
            temp = t->right;
            free(t);
            return temp;
        }

        if (t->right == NULL)
        {
            temp = t->left;
            free(t);
            return temp;
        }

        /* Case 3 : node has two children - use inorder successor */
        temp = t->right;

        while (temp->left != NULL)
        {
            temp = temp->left;
        }

        t->id   = temp->id;
        t->item = temp->item;
        t->right = deleteBST(t->right, temp->id);
    }

    return t;
}


/* =====================================================================
 * SECTION 4 : MODULE 4 - HASH TABLE (Linear Probing)
 * Stores claimant details; key = claim ID
 * ===================================================================== */

enum { EMPTY, OCCUPIED, DELETED };

struct Claimant
{
    int claimId;
    int itemId;
    int state;
    char name[LEN];
};

/* Global array, so every slot starts as 0 (EMPTY) */
struct Claimant table[HASH_SIZE];

/* 4.1 Search for a claim ID; returns slot index or -1 */
int searchHash(int key)
{
    int i, idx;

    for (i = 0; i < HASH_SIZE; i++)
    {
        idx = (key + i) % HASH_SIZE;

        if (table[idx].state == EMPTY)
        {
            return -1;
        }

        if (table[idx].state == OCCUPIED && table[idx].claimId == key)
        {
            return idx;
        }
    }

    return -1;
}

/* 4.2 Insert claimant details; returns slot index or -1 if full */
int insertHash(int key, int itemId, char name[])
{
    int i, idx;

    for (i = 0; i < HASH_SIZE; i++)
    {
        idx = (key + i) % HASH_SIZE;

        if (table[idx].state != OCCUPIED)
        {
            table[idx].claimId = key;
            table[idx].itemId  = itemId;
            strcpy(table[idx].name, name);
            table[idx].state   = OCCUPIED;
            return idx;
        }
    }

    return -1;
}


/* =====================================================================
 * SECTION 5 : SYSTEM OPERATIONS
 * ===================================================================== */

/* 5.1 Staff reports a found item (Linked List + BST) */
void reportItem()
{
    char name[LEN];
    int id = readInt("\nEnter Item ID: ");

    if (searchBST(root, id) != NULL)
    {
        printf("Item ID already exists.\n");
        return;
    }

    printf("Enter Item Name: ");
    scanf(" %49[^\n]", name);

    root = insertBST(root, id, insertItem(id, name));

    printf("Item %d added.\n", id);
}

/* 5.2 Passenger searches for an item (BST) */
void searchItem()
{
    int id = readInt("\nEnter Item ID: ");
    struct Item *it = searchBST(root, id);

    if (it != NULL)
    {
        printf("Found -> ID: %d | Item: %s\n", it->id, it->name);
    }
    else
    {
        printf("Item not found.\n");
    }
}

/* 5.3 Passenger files a claim (Hash Table + Queue) */
void fileClaim()
{
    char name[LEN];
    int claimId, itemId;

    if (count == MAX_QUEUE)
    {
        printf("\nClaim queue full.\n");
        return;
    }

    claimId = readInt("\nEnter Claim ID: ");

    if (searchHash(claimId) != -1)
    {
        printf("Claim ID already pending.\n");
        return;
    }

    itemId = readInt("Enter Item ID being claimed: ");

    if (searchBST(root, itemId) == NULL)
    {
        printf("No such unclaimed item.\n");
        return;
    }

    printf("Enter Passenger Name: ");
    scanf(" %49[^\n]", name);

    if (insertHash(claimId, itemId, name) == -1)
    {
        printf("Claim table full.\n");
        return;
    }

    enqueue(claimId);

    printf("Claim %d added to queue.\n", claimId);
}

/* 5.4 Admin processes the next claim (Queue + Hash + BST + List) */
void processClaim()
{
    int idx, id;
    struct Item *it;

    if (count == 0)
    {
        printf("\nNo pending claims.\n");
        return;
    }

    idx = searchHash(dequeue());
    it  = searchBST(root, table[idx].itemId);

    printf("\nClaim %d | Passenger: %s | Item ID: %d\n",
           table[idx].claimId, table[idx].name, table[idx].itemId);

    if (it != NULL)
    {
        printf("APPROVED: '%s' handed over.\n", it->name);

        id = it->id;
        root = deleteBST(root, id);
        deleteItem(id);
    }
    else
    {
        printf("REJECTED: item already handed over.\n");
    }

    table[idx].state = DELETED;
}


/* =====================================================================
 * SECTION 6 : MAIN FUNCTION (MENU DRIVER)
 * ===================================================================== */

int main()
{
    while (1)
    {
        printf("\n===== LOST ITEM RECOVERY SYSTEM =====\n");
        printf("1. Report Found Item\n");
        printf("2. Search Item\n");
        printf("3. View All Items\n");
        printf("4. File Claim\n");
        printf("5. Process Next Claim\n");
        printf("6. Exit\n");

        switch (readInt("Enter choice: "))
        {
            case 1:
                reportItem();
                break;

            case 2:
                searchItem();
                break;

            case 3:
                displayItems();
                break;

            case 4:
                fileClaim();
                break;

            case 5:
                processClaim();
                break;

            case 6:
                return 0;

            default:
                printf("Invalid choice.\n");
        }
    }
}
