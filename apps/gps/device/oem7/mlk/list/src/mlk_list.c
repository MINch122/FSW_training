#include "mlk_list_impl.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static void mlk_list_purge(mlk_list_t l) {
    if (l) {
        memset(l, 0, sizeof(*l));
    }
}

/**
 * Internal helper to get the tail node. Returns NULL if the list is 
 * not sane or no node is attached. tailPrev will be set, if passed,
 * to the node before the tail, or NULL if the list has a single node.
 */
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
static mlk_list_node_t* get_tail(const mlk_list_t list) {
#else
static mlk_list_node_t* get_tail(const mlk_list_t list,
                                 mlk_list_node_t** tailPrev) {
#endif
    mlk_list_node_t* t = NULL;
    if (list && list->head) {
        t = list->accessing ? list->accessing : list->head;
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
#else 
        if (tailPrev) {
            *tailPrev = NULL;
        }
#endif
        while (t->next) {
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
#else 
            if (tailPrev) {
                *tailPrev = t;
            }
#endif
            t = t->next;
        }
    }
    return t;
}

static mlk_list_node_t* mlk_list_node(size_t size) {
    mlk_list_node_t* n = NULL;
    /**
     * The data block follows the header struct.
     */
    if ((n = malloc(NODE_ALLOC_SZ(size))) != NULL) {
        /**
         * Zero data size is not considered an error.
         * Just allocate NULL so that the node can be
         * later used as a simple pointer container.
         */
        n->data = size > 0 ? n + 1 : NULL;
        n->size = size;
    }
    return n;
}

static void mlk_list_node_free(mlk_list_node_t* n) {
    if (n)
        free(n);
}



mlk_list_t mlk_list(void) {
    mlk_list_t l = malloc(sizeof(*l));
    mlk_list_purge(l);
    return l;
}

void mlk_list_free(mlk_list_t list) {
    mlk_list_node_t* n;
    if (!list) {
        return;
    }
    n = list->head;
    while (n) {
        list->head = n;
        n = n->next;
        mlk_list_node_free(list->head);
    }
    free(list);
}

bool mlk_list_to_head(mlk_list_t list) {
    if (list && list->head) {
        list->accessing = list->head;
        return true;
    }
    return false;
}

bool mlk_list_to_tail(mlk_list_t list) {
    mlk_list_node_t* n;
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    if ((n = get_tail(list)) == NULL) {
#else
    if ((n = get_tail(list, NULL)) == NULL) {
#endif
        return false;
    }
    list->accessing = n;
    return true;
}

bool mlk_list_advance(mlk_list_t list) {
    if (list && list->accessing && list->accessing->next) {
        list->accessing = list->accessing->next;
        return true;
    }
    return false;
}

#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
bool mlk_list_retreat(mlk_list_t list) {
    if (list && list->accessing && list->accessing->prev) {
        list->accessing = list->accessing->prev;
        return true;
    }
    return false;
}
#endif

bool mlk_list_is_head(const mlk_list_t list) {
    return list && 
           list->accessing && 
           list->accessing == list->head ?
               true :
               false;
}

bool mlk_list_is_tail(const mlk_list_t list) {
    return list &&
           list->accessing &&
           !list->accessing->next ?
               true :
               false;
}

int mlk_list_nodes(const mlk_list_t list) {
    return list ? list->nodes : M_NULL;
}

size_t mlk_list_node_size(const mlk_list_t list) {
    return list && list->accessing ? list->accessing->size : 0;
}

void* mlk_list_node_data(const mlk_list_t list) {
    return list && list->accessing ? list->accessing->data : NULL;
}

int mlk_list_node_dump_partial(const mlk_list_t list,
                               void* data,
                               size_t size,
                               int offset) {
    if (!list || !data) {
        return M_NULL;
    }
    if (!list->accessing) {
        return M_LIST_NCURNODE;
    }
    if ((size_t) offset + size > list->accessing->size) {
        return M_SIZE;
    }
    memcpy(data, ((uint8_t*) list->accessing->data) + offset, size);
    return M_SUCCESS;
}

int mlk_list_node_dump(const mlk_list_t list,
                       void* data) {
    return mlk_list_node_dump_partial(list,
                                      data,
                                      list->accessing->size,
                                      0);
}

int mlk_list_add_front(mlk_list_t list,
                       const void* data,
                       size_t size) {
    mlk_list_node_t* n;

    if (!list) {
        return M_NULL;
    }

    if ((n = mlk_list_node(size)) == NULL) {
        return M_NOMEM;
    }

    if (size == 0) {
        n->data = (void*)data;
    }
    else {
        memcpy(n->data, data, size);
    }
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    if (list->head) {
        list->head->prev = n;
    }
#endif
    n->next = list->head;
    list->head = n;
    if (!list->accessing) {
        list->accessing = n;
    }
    list->nodes++;

    return M_SUCCESS;
}

int mlk_list_add_back(mlk_list_t list,
                      const void* data,
                      size_t size) {
    mlk_list_node_t* n;

    if (!list) {
        return M_NULL;
    }

    if ((n = mlk_list_node(size)) == NULL) {
        return M_NOMEM;
    }

    if (size == 0) {
        n->data = (void*)data;
    }
    else {
        memcpy(n->data, data, size);
    }
    n->next = NULL;
    if (list->head == NULL) {
        list->head = n;
        list->accessing = n;
    }
    else {
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
        n->prev = get_tail(list);
        n->prev->next = n;
#else
        get_tail(list, NULL)->next = n;
#endif
    }
    list->nodes++;
    return M_SUCCESS;
}

int mlk_list_add_insert(mlk_list_t list,
                        int index,
                        const void* data,
                        size_t size) {
    mlk_list_node_t* n;
    mlk_list_node_t* front;

    if (!list) {
        return M_NULL;
    }

    if (index > mlk_list_nodes(list)) {
        return M_RANGE;
    }

    if ((n = mlk_list_node(size)) == NULL) {
        return M_NOMEM;
    }
    size = size > 0 ? size : sizeof(data);
    memcpy(n->data, data, size);

    if (index == 0) {
        n->next = list->head;
        list->head = n;
    }
    else {
        front = list->head;
        for (int i = 1; i < index; ++i) {
            front = front->next;
        }
        n->next = front->next;
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
        n->prev = front;
        front->next->prev = n;
#endif
        front->next = n;
    }

    list->nodes++;

    return M_SUCCESS;

}

int mlk_list_pop_front(mlk_list_t list,
                       void* data) {
    mlk_list_node_t* h;
    if (!list) {
        return M_NULL;
    }
    if (!list->head) {
        return M_LIST_NHEAD;
    }
    h = list->head;
    if (data) {
        if (!h->data) {
            return M_LIST_NDATA;
        }
        memcpy(data, h->data, h->size);
    }
    list->head = list->head->next;
    if (list->accessing == h) {
        list->accessing = list->head;
    }
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    if (list->head) {
        list->head->prev = NULL;
    }
#endif
    mlk_list_node_free(h);
    list->nodes--;
    return M_SUCCESS;
}

int mlk_list_pop_back(mlk_list_t list,
                      void* data) {
    mlk_list_node_t* t;
    mlk_list_node_t* prev;
    if (!list) {
        return M_NULL;
    }
    if (!list->head) {
        return M_LIST_NHEAD;
    }
    if (list->head->next == NULL) {
        /**
         * If there is a single node here, cut the head.
         */
        t = list->head;
        list->head = NULL;
    }
    else {
        /**
         * Else we traverse to the end.
         */
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
        if ((t = get_tail(list)) == NULL) {
            return M_LIST_UNKNOWN;
        }
        prev = t->prev;
#else
        if ((t = get_tail(list, &tprev)) == NULL) {
            return M_LIST_UNKNOWN;
        }
#endif
        if (prev) {
            t->prev->next = NULL;
        }
    }

    if (data) {
        if (!t->data) {
            return M_LIST_NDATA;
        }
        memcpy(data, t->data, t->size);
    }
    if (t == list->accessing) {
        list->accessing = prev;
    }
    mlk_list_node_free(t);
    list->nodes--;
    return M_SUCCESS;
}

int mlk_list_pop_current(mlk_list_t list,
                         void* data) {
    mlk_list_node_t* prev;

    if (!list) {
        return M_NULL;
    }
    if (!list->accessing) {
        return M_LIST_NCURNODE;
    }

    if (mlk_list_is_head(list)) {
        return mlk_list_pop_front(list, data);
    }

    if (mlk_list_is_tail(list)) {
        return mlk_list_pop_back(list, data);
    }

#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    prev = list->accessing->prev;
#else
    prev = list->head;
    while (prev != NULL && prev->next != list->accessing) {
        prev = list->head;
    }
#endif
    /* This must have been caught by the earlier mlk_list_is_tail(). */
    if (prev == NULL) {
        return M_UNKNOWN;
    }

    if (data) {
        if (!list->accessing->data) {
            return M_NODATA;
        }
        memcpy(data, list->accessing->data, list->accessing->size);
    }
    prev->next = list->accessing->next;
#if defined(MLK_LIST_DOUBLY_LINKED) && MLK_LIST_DOUBLY_LINKED == 1
    if (list->accessing->next) {
        list->accessing->next->prev = prev;
    }
#endif
    mlk_list_node_free(list->accessing);
    list->accessing = prev->next ? prev->next : prev;
    list->nodes--;

    return M_SUCCESS;
}

int mlk_list_iterate(mlk_list_t list,
                     int (*func)(void*, void*),
                     void* args,
                     bool escape) {
    mlk_list_node_t* accessing;
    if (!list || !func) {
        return M_NULL;
    }
    if (list->nodes == 0) {
        return M_SUCCESS;
    }
    /**
     * If we don't care about the iterator results, restore
     * the accessing node position after we are through.
     */
    if (!escape) {
        accessing = list->accessing;
    }
    if (mlk_list_to_head(list)) {
        do {
            if (func(mlk_list_node_data(list), args) != 0 && escape) {
                return 1;
            }
        } while (mlk_list_advance(list));
    }
    if (!escape && accessing) {
        list->accessing = accessing;
    }
    return M_SUCCESS;
}


#include <stdio.h>
void mlk_list_print_all(const mlk_list_t list, int line, const char* caller) {
    mlk_list_node_t* n;
    if (!list) {
        printf("%s (line: %d, caller: %s): list is null.\n",
               __func__,
               line,
               caller);
        return;
    }
    printf("%s (line: %d, caller: %s): nodes %d, head %p, accessing %p.\n",
           __func__,
           line,
           caller,
           list->nodes,
           (const void*)list->head,
           (const void*)list->accessing);
    if (!list->head) {
        return;
    }
    n = list->head;
    for (int i = 0; i < list->nodes; ++i) {
        printf("\tnode %" PRIi32 "(%" PRIXPTR "): size %zu, data @ %" PRIXPTR ", next %" PRIXPTR "\n",
               i,
               (uintptr_t)n,
               n->size,
               (uintptr_t)n->data,
               (uintptr_t)n->next);
        n = n->next;
    }
}
