#ifndef _MLK_LIST_H_
#define _MLK_LIST_H_

#include <mlk_cmtypes.h>

#define MLK_LIST_DOUBLY_LINKED 1

typedef enum {
    M_LIST_ERR          = M_LIST_PHMIN,
    M_LIST_NHEAD        = M_LIST_ERR - 1,   /* List has no head. */
    M_LIST_NCURNODE     = M_LIST_ERR - 2,   /* List has no accessing node. */
    M_LIST_NDATA        = M_LIST_ERR - 3,   /* Node has null data pointer. */
    M_LIST_UNKNOWN      = M_LIST_ERR - 4,
} mlk_list_ret_t;

#if M_LIST_UNKNOWN < M_LIST_PHMAX 
  #error "LIST-specific error codes exceeds allocated retcode range. Fix mlk_list_ret_t."
#endif


/**
 * An opaque handle for linked-list objects.
 */
typedef struct mlk_list_s* mlk_list_t;


/**
 * @brief Create an empty linked-list.
 * 
 * @return A pointer to the new list object, or NULL if fails.
 */
mlk_list_t mlk_list(void);


void mlk_list_free(mlk_list_t list);

bool mlk_list_to_head(mlk_list_t list);
bool mlk_list_to_tail(mlk_list_t list);
bool mlk_list_advance(mlk_list_t list);
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
bool mlk_list_retreat(mlk_list_t list);
#endif
bool mlk_list_is_head(const mlk_list_t list);
bool mlk_list_is_tail(const mlk_list_t list);

int mlk_list_nodes(const mlk_list_t list);
size_t mlk_list_node_size(const mlk_list_t list);
void* mlk_list_node_data(const mlk_list_t list);

int mlk_list_node_dump(const mlk_list_t list,
                       void* data);

int mlk_list_node_dump_partial(const mlk_list_t list,
                               void* data,
                               size_t size,
                               int offset);

int mlk_list_add_front(mlk_list_t list,
                       const void* data,
                       size_t size);

int mlk_list_add_back(mlk_list_t list,
                      const void* data,
                      size_t size);

int mlk_list_add_insert(mlk_list_t list,
                        int index,
                        const void* data,
                        size_t size);

#define mlk_list_add(list, data, size) mlk_list_add_back(list, data, size)

int mlk_list_pop_front(mlk_list_t list,
                       void* data);
int mlk_list_pop_back(mlk_list_t list,
                      void* data);
int mlk_list_pop_current(mlk_list_t list,
                         void* data);

#define mlk_list_add_ptr_insert(list, index, ptr)   mlk_list_add_insert(list, index, ptr, 0)
#define mlk_list_add_ptr_front(list, ptr)           mlk_list_add_front(list, ptr, 0)
#define mlk_list_add_ptr_back(list, ptr)            mlk_list_add_back(list, ptr, 0)
#define mlk_list_add_ptr(list, ptr)                 mlk_list_add(list, ptr, 0)

/**
 * @brief Iterate a function over the given list.
 * 
 * @details From head to tail, execute @a func for each node data in @a list.
 *          If @a escape is true, the iteration stops immediately when @a func 
 *          returns any nonzero value, setting the accessing node to the one 
 *          that stopped iteration.
 *          If @a escape is false, @a func is executed for every node, 
 *          regargless of hte return values. After that, the accessing node is 
 *          restored to anything it was before the iteration.
 * 
 * @param[in] list A valid list. 
 * @param[in] func A function to be executed: func(node_data, args).
 * @param[in] args Additional argument to pass to @a func.
 * @param[in] escape If true, iteration stops when func returns nonzero. If
 *                   false, return value is not checked (runs over all nodes).
 * @return int 
 */
int mlk_list_iterate(mlk_list_t list,
                     int (*func)(void* nodedata, void* arg),
                     void* args,
                     bool escape);

void mlk_list_print_all(const mlk_list_t list, int line, const char* func);
#define mlk_list_print(list) mlk_list_print_all(list, __LINE__, __func__)

#endif
