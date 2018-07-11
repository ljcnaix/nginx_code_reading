
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef void *            ngx_buf_tag_t;

typedef struct ngx_buf_s  ngx_buf_t;

struct ngx_buf_s {
    // 通常是告诉使用者，从pos位置处开始处理内存中的数据
    u_char          *pos;
    // last表示有效的内存到此为止, pos与last之间的内容是希望nginx处理的内容
    u_char          *last;
    // 处理文件时，file_pos与file_last是与pos和last相对应的概念
    off_t            file_pos;
    off_t            file_last;

    // 如果ngx_buf_s用于内存，那么start指向这段内存的起始地址
    u_char          *start;
    // 与start成员相对应，指向缓冲区内存的末尾
    u_char          *end;

    // 表示当前缓冲区的类型，列如哪个模块使用就指向该模块ngx_module_t变量的地址
    ngx_buf_tag_t    tag;
    // 引用的文件
    ngx_file_t      *file;
    // 比较复杂暂时不用管
    ngx_buf_t       *shadow;

    // 临时内存标志位，为1时表示数据在内存中，且这段内存可以修改
    unsigned         temporary:1;

    // 标志位，为1时表示数据在内存中，且这段内存不可以修改
    unsigned         memory:1;

    // 标志位，为1时表示这段内存是通过mmap映射过来的，不可以修改
    unsigned         mmap:1;

    // 标志位，为1时表示可回收
    unsigned         recycled:1;

    // 标志位，为1时表示这段缓冲区处理的是文件，而不是内存
    unsigned         in_file:1;

    // 标志位，为1时表示需要执行flush操作
    unsigned         flush:1;

    //
    unsigned         sync:1;

    // 标志位，表示是否是最后一块缓冲区
    unsigned         last_buf:1;

    // 标志位，表示是否是ngx_chain_t中最后一块缓冲区
    unsigned         last_in_chain:1;

    //
    unsigned         last_shadow:1;

    // 标志位，表示当前缓冲区是否属于临时文件
    unsigned         temp_file:1;

    /* STUB */ int   num;
};

// ngx_chain_t是与ngx_buf_t配合使用的链表数据结构
struct ngx_chain_s {
    ngx_buf_t    *buf;
    ngx_chain_t  *next;
};


typedef struct {
    ngx_int_t    num;
    size_t       size;
} ngx_bufs_t;


typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;

typedef ngx_int_t (*ngx_output_chain_filter_pt)(void *ctx, ngx_chain_t *in);

#if (NGX_HAVE_FILE_AIO)
typedef void (*ngx_output_chain_aio_pt)(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif

struct ngx_output_chain_ctx_s {
    ngx_buf_t                   *buf;
    ngx_chain_t                 *in;
    ngx_chain_t                 *free;
    ngx_chain_t                 *busy;

    unsigned                     sendfile:1;
    unsigned                     directio:1;
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    unsigned                     unaligned:1;
#endif
    unsigned                     need_in_memory:1;
    unsigned                     need_in_temp:1;
#if (NGX_HAVE_FILE_AIO)
    unsigned                     aio:1;

    ngx_output_chain_aio_pt      aio_handler;
#endif

    off_t                        alignment;

    ngx_pool_t                  *pool;
    ngx_int_t                    allocated;
    ngx_bufs_t                   bufs;
    ngx_buf_tag_t                tag;

    ngx_output_chain_filter_pt   output_filter;
    void                        *filter_ctx;
};


typedef struct {
    ngx_chain_t                 *out;
    ngx_chain_t                **last;
    ngx_connection_t            *connection;
    ngx_pool_t                  *pool;
    off_t                        limit;
} ngx_chain_writer_ctx_t;


#define NGX_CHAIN_ERROR     (ngx_chain_t *) NGX_ERROR


#define ngx_buf_in_memory(b)        (b->temporary || b->memory || b->mmap)
#define ngx_buf_in_memory_only(b)   (ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_special(b)                                                   \
    ((b->flush || b->last_buf || b->sync)                                    \
     && !ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_sync_only(b)                                                 \
    (b->sync                                                                 \
     && !ngx_buf_in_memory(b) && !b->in_file && !b->flush && !b->last_buf)

#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) (b->last - b->pos):                      \
                            (b->file_last - b->file_pos))

ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);


#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t))

ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);
#define ngx_free_chain(pool, cl)                                             \
    cl->next = pool->chain;                                                  \
    pool->chain = cl



ngx_int_t ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in);
ngx_int_t ngx_chain_writer(void *ctx, ngx_chain_t *in);

ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in);
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);
void ngx_chain_update_chains(ngx_chain_t **free, ngx_chain_t **busy,
    ngx_chain_t **out, ngx_buf_tag_t tag);


#endif /* _NGX_BUF_H_INCLUDED_ */
