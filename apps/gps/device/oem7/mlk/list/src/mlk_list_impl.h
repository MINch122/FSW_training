#include <mlk_list.h>

struct mlk_list_node_s;
typedef struct mlk_list_node_s mlk_list_node_t;

struct mlk_list_node_s {
    mlk_list_node_t* next;
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    mlk_list_node_t* prev;
#endif
    size_t size;
    void* data;
};
#define a sizeof(struct mlk_list_node_s)

struct mlk_list_s {
    mlk_list_node_t* head;
    mlk_list_node_t* accessing;
    int nodes;
};

#define NODE_ALLOC_SZ(s)        ((s) + sizeof(mlk_list_node_t))
