
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*
 * 下面的代码定义了nginx链表类型ngx_list_t
 */

// ngx_list_part_t是链表的节点类型，描述一个节点包含的信息
typedef struct ngx_list_part_s  ngx_list_part_t;

struct ngx_list_part_s {
    void             *elts;   // 指向该节点数组的起始地址
    ngx_uint_t        nelts;  // 表示数组中已经使用了多少个元素, nelts <= nalloc
    ngx_list_part_t  *next;   // 下一个链表节点的地址
};

typedef struct {
    ngx_list_part_t  *last;   // 指向链表的最后一个数组元素
    ngx_list_part_t   part;   // 指向链表的首个数组元素
    size_t            size;   // 链表节点数组元素大小的上限
    ngx_uint_t        nalloc; // 链表节点数组元素个数的上限
    ngx_pool_t       *pool;   // 链表中管理内存的内存池对象
} ngx_list_t;

// 用于创建新链表
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

// 用于初始化链表
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */

// 用于添加新元素
void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
