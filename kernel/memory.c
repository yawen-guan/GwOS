#include "memory.h"

#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "print.h"
#include "string.h"
#include "sync.h"
#include "thread.h"

#define MEM_BITMAP_BASE 0xc009a000    // 支持四页的位图，最大可管理512MB的物理内存
#define KERNEL_HEAP_START 0xc0100000  // 堆内存：3GB + 1MB开始

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)  // 返回虚拟地址的高10位
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)  // 返回虚拟地址的中间10位

// 内存池
struct pool {
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start;  // 该内存池管理的物理内存起始地址
    uint32_t pool_size;       // 该内存池的容量（字节数）
    struct lock lock;
};

struct arena {
    struct mem_block_desc *desc;
    uint32_t cnt;  // large == true, cnt = 页框数; large == false, cnt = 内存块数
    bool large;
};

struct mem_block_desc kernel_block_descs[DESC_CNT];
struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;

/**
 * @brief 在pf表示的虚拟内存池中申请cnt个虚拟页
 * 
 * @return 成功：返回虚拟页的起始地址； 失败：返回NULL
 */
void *vaddr_get(enum pool_flag pf, uint32_t cnt) {
    int vaddr_start = 0, idx_bit_start = -1;
    uint32_t count = 0;
    if (pf == PF_KERNEL) {  //kernel_pool
        idx_bit_start = bitmap_apply_cnt(&kernel_vaddr.vaddr_bitmap, cnt);
        if (idx_bit_start == -1) return NULL;

        while (count < cnt) {
            bitmap_set_idx(&kernel_vaddr.vaddr_bitmap, idx_bit_start + count, 1);
            count++;
        }
        vaddr_start = kernel_vaddr.vaddr_start + idx_bit_start * PG_SIZE;
    } else {  // user_pool
        struct pcb *now = running_thread();
        idx_bit_start = bitmap_apply_cnt(&now->userprog_vaddr.vaddr_bitmap, cnt);
        if (idx_bit_start == -1) return NULL;
        while (count < cnt) {
            bitmap_set_idx(&now->userprog_vaddr.vaddr_bitmap, idx_bit_start + count, 1);
            count++;
        }
        vaddr_start = now->userprog_vaddr.vaddr_start + idx_bit_start * PG_SIZE;

        //start_process中占用了0xc0000000 - PG_SIZE
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));
    }
    return (void *)vaddr_start;
}

/**
 * @brief 求虚拟地址vaddr对应的pte指针
 * 
 * @return pte指针
 */
uint32_t *pte_ptr(uint32_t vaddr) {
    //  0xffc00000让处理器在最后一个pde中取出页目录表的物理地址
    uint32_t pde_idx = vaddr & 0xffc00000;  // 高10位,后面0
    uint32_t *pte = (uint32_t *)(0xffc00000 + (pde_idx >> 10) + (PTE_IDX(vaddr) * 4));
    return pte;
}

/**
 * @brief 求虚拟地址vaddr对应的pde指针
 * 
 * @return pde指针
 */
uint32_t *pde_ptr(uint32_t vaddr) {
    //  0xfffffxxx获得页目录表的物理地址
    uint32_t *pde = (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);
    return pde;
}

/**
 * @brief page allocate 在p指向的物理内存池中分配一个物理页
 * 
 * @return 成功：返回页框的物理地址；失败：返回NULL
 */
void *palloc(struct pool *p) {
    int idx = bitmap_apply_cnt(&p->pool_bitmap, 1);
    if (idx == -1) return NULL;
    bitmap_set_idx(&p->pool_bitmap, idx, 1);

    uint32_t page_phyaddr = (idx * PG_SIZE) + p->phy_addr_start;
    return (void *)page_phyaddr;
}

/** 
 * @brief 在页表中添加虚拟地址vaddr和物理大致page_phyaddr的映射
 * 
 */
void page_table_add(void *_vaddr, void *_page_phyaddr) {
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pde = pde_ptr(vaddr);
    uint32_t *pte = pte_ptr(vaddr);

    if (*pde & PG_P_1) {           // 该页目录项已存在
        ASSERT(!(*pte & PG_P_1));  // 该页表项不存在
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

    } else {
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);  // pde_phyaddr对应的物理内存页清空

        ASSERT(!(*pte & PG_P_1));  // 该页表项不存在
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/**
 * @brief 在pf对应的内存池中分配cnt个页
 * 
 * @return 成功：返回起始虚拟地址；失败：返回NULL 
 */
void *malloc_page(enum pool_flag pf, uint32_t cnt) {
    ASSERT(cnt > 0 && cnt < 3840);  // 一个内存池16MB，15MB=3840页，限制其不超过3840页

    // 申请虚拟地址
    void *vaddr_start = vaddr_get(pf, cnt);
    if (vaddr_start == NULL) return NULL;

    // 从内存池中申请物理页
    uint32_t vaddr = (uint32_t)vaddr_start;
    struct pool *p = (pf == PF_KERNEL) ? &kernel_pool : &user_pool;

    for (int i = 0; i < cnt; i++) {
        void *page_phyaddr = palloc(p);
        if (page_phyaddr == NULL) return NULL;
        page_table_add((void *)vaddr, page_phyaddr);
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

/**
 * @brief 从内核内存池中申请cnt页
 * 
 * @return 成功：返回虚拟地址；失败：返回NULL
 */
void *get_kernel_pages(uint32_t cnt) {
    lock_acquire(&kernel_pool.lock);
    void *vaddr = malloc_page(PF_KERNEL, cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, cnt * PG_SIZE);
    }
    lock_release(&kernel_pool.lock);
    return vaddr;
}

/**
 * @brief 从用户内存池中申请cnt页
 * 
 * @return 成功：返回虚拟地址；失败：返回NULL
 */
void *get_user_pages(uint32_t cnt) {
    lock_acquire(&user_pool.lock);
    void *vaddr = malloc_page(PF_USER, cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, cnt * PG_SIZE);
    }
    lock_release(&user_pool.lock);
    return vaddr;
}

/**
 * @brief 在pf对应的内存池中申请一页内存，并将虚拟地址vaddr映射到这一页（指定虚拟地址）
 * 
 * @param pf 
 * @param vaddr 
 * @return void* 
 */
void *get_one_page(enum pool_flag pf, uint32_t vaddr) {
    struct pool *p = (pf == PF_KERNEL) ? &kernel_pool : &user_pool;
    lock_acquire(&p->lock);

    struct pcb *now = running_thread();
    int32_t idx = -1;

    //用户进程
    if (now->pg_dir != NULL && pf == PF_USER) {
        ASSERT(vaddr > now->userprog_vaddr.vaddr_start);
        idx = (vaddr - now->userprog_vaddr.vaddr_start) / PG_SIZE;
        bitmap_set_idx(&now->userprog_vaddr.vaddr_bitmap, idx, 1);
    }
    //内核线程
    else if (now->pg_dir == NULL && pf == PF_KERNEL) {
        ASSERT(vaddr > kernel_vaddr.vaddr_start);
        idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        bitmap_set_idx(&kernel_vaddr.vaddr_bitmap, idx, 1);
    }
    //其他情况不允许
    else {
        PANIC("get one page: panic");
    }

    void *page_phyaddr = palloc(p);
    if (page_phyaddr == NULL) {
        lock_release(&p->lock);
        return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    lock_release(&p->lock);
    return (void *)vaddr;
}

/**
 * @brief 为vaddr分配一页物理页，但不需要从虚拟地址内存池中设置位图
 * 
 * @return void* 
 */
void *get_one_page_no_bitmap(enum pool_flag pf, uint32_t vaddr) {
    struct pool *p = (pf == PF_KERNEL) ? &kernel_pool : &user_pool;
    lock_acquire(&p->lock);
    void *page_phyaddr = palloc(p);
    if (page_phyaddr == NULL) {
        lock_release(&p->lock);
        return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    lock_release(&p->lock);
    return (void *)vaddr;
}

/**
 * @brief 将虚拟地址转换为物理地址
 * 
 * @return uint32_t 
 */
uint32_t addr_vir2phy(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    return ((*pte & 0xFFFFF000) + (vaddr & 0x00000FFF));
}

/**
 * @brief 返回arena中第idx个内存块的地址
 * 
 * @param a 
 * @param idx 
 * @return struct mem_block* 
 */
struct mem_block *arena2block(struct arena *a, uint32_t idx) {
    return (struct mem_block *)((uint32_t)a + sizeof(struct arena) + idx * a->desc->block_size);
}

/**
 * @brief 返回内存块b所在的arena的地址
 * 
 */
struct arena *block2arena(struct mem_block *b) {
    return (struct arena *)((uint32_t)b & 0xfffff000);
}

/**
 * @brief malloc申请size字节的内存（在堆中）
 * 
 * @param size 
 * @return void* 
 */
void *sys_malloc(uint32_t size) {
    enum pool_flag pf;
    struct pool *p;
    uint32_t pool_size;
    struct mem_block_desc *descs;
    struct pcb *thread = running_thread();

    if (thread->pg_dir == NULL) {  // 内核线程
        pf = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        p = &kernel_pool;
        descs = kernel_block_descs;
    } else {  // 用户进程
        pf = PF_USER;
        pool_size = user_pool.pool_size;
        p = &user_pool;
        descs = thread->user_block_descs;
    }

    if (!(size > 0 && size < pool_size)) return NULL;

    struct arena *a;
    struct mem_block *b;
    lock_acquire(&p->lock);

    if (size > 1024) {  //size > 1024, 分配页框
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PG_SIZE);
        a = malloc_page(pf, page_cnt);
        if (a == NULL) {
            lock_release(&p->lock);
            return NULL;
        }
        memset(a, 0, page_cnt * PG_SIZE);
        a->desc = NULL;
        a->cnt = page_cnt;
        a->large = true;
        lock_release(&p->lock);
        return (void *)(a + 1);
    }

    //size <= 1024

    uint8_t desc_idx;
    for (int i = 0; i < DESC_CNT; i++) {
        desc_idx = i;
        if (size <= descs[i].block_size) {  // 从小往大后,找到后退出
            break;
        }
    }

    if (list_empty(&descs[desc_idx].free_list)) {
        a = malloc_page(pf, 1);
        if (a == NULL) {
            lock_release(&p->lock);
            return NULL;
        }
        memset(a, 0, PG_SIZE);
        a->desc = &descs[desc_idx];
        a->large = false;
        a->cnt = descs[desc_idx].blocks_per_arena;
        uint32_t block_idx;
        enum intr_status old_status = intr_disable();
        for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena; block_idx++) {
            b = arena2block(a, block_idx);
            ASSERT(!node_find(&a->desc->free_list, &b->free_node));
            list_append(&a->desc->free_list, &b->free_node);
        }
        intr_set_status(old_status);
    }

    b = elem2entry(struct mem_block, free_node, list_pop(&(descs[desc_idx].free_list)));
    memset(b, 0, descs[desc_idx].block_size);

    a = block2arena(b);
    a->cnt--;
    lock_release(&p->lock);
    return (void *)b;
}

/**
 * @brief page pfree 在物理内存池中释放一个物理页
 * 
 */
void pfree(uint32_t pg_phy_addr) {
    struct pool *p;
    uint32_t idx = 0;
    if (pg_phy_addr >= user_pool.phy_addr_start) {  // user pool
        p = &user_pool;
        idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
    } else {  // kernel pool
        p = &kernel_pool;
        idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
    }
    bitmap_set_idx(&p->pool_bitmap, idx, 0);
}

/**
 * @brief 在页表中去掉虚拟地址的映射
 * 
 * @param vaddr 
 */
void page_table_pte_remove(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;
    asm volatile("invlpg %0" ::"m"(vaddr)
                 : "memory");
}

/**
 * @brief 在虚拟内存池中释放vaddr开头的连续cnt个虚拟页地址
 * 
 * @param pf 
 * @param void_vaddr 
 * @param cnt 
 */
void vaddr_remove(enum pool_flag pf, void *void_vaddr, uint32_t cnt) {
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)void_vaddr, count = 0;

    if (pf == PF_KERNEL) {  // kernel
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        while (count < cnt) {
            bitmap_set_idx(&kernel_vaddr.vaddr_bitmap, bit_idx_start + count++, 0);
        }
    } else {  // user
        struct pcb *thread = running_thread();
        bit_idx_start = (vaddr - thread->userprog_vaddr.vaddr_start) / PG_SIZE;
        while (count < cnt) {
            bitmap_set_idx(&thread->userprog_vaddr.vaddr_bitmap, bit_idx_start + count++, 0);
        }
    }
}

/**
 * @brief 在物理地址池中释放物理页地址、在页表中去掉虚拟地址的映射、在虚拟地址池中释放虚拟地址：释放vaddr开头的连续cnt个物理页
 * 
 */
void mfree_page(enum pool_flag pf, void *void_vaddr, uint32_t cnt) {
    uint32_t vaddr = (int32_t)void_vaddr, page_cnt = 0;
    ASSERT(cnt >= 1 && vaddr % PG_SIZE == 0);
    uint32_t pg_phy_addr = addr_vir2phy(vaddr);

    ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= 0x102000);

    if (pg_phy_addr >= user_pool.phy_addr_start) {  // user
        vaddr -= PG_SIZE;
        while (page_cnt < cnt) {
            vaddr += PG_SIZE;
            pg_phy_addr = addr_vir2phy(vaddr);
            ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= user_pool.phy_addr_start);
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, void_vaddr, cnt);

    } else {  // kernel
        vaddr -= PG_SIZE;
        while (page_cnt < cnt) {
            vaddr += PG_SIZE;
            pg_phy_addr = addr_vir2phy(vaddr);
            /* 确保待释放的物理内存只属于内核物理内存池 */
            ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= kernel_pool.phy_addr_start && pg_phy_addr < user_pool.phy_addr_start);
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, void_vaddr, cnt);
    }
}

/**
 * @brief 释放ptr所指向的内存
 * 
 * @param ptr 
 */
void sys_free(void *ptr) {
    ASSERT(ptr != NULL);
    if (ptr == NULL) return;
    enum pool_flag pf;
    struct pool *p;

    if (running_thread()->pg_dir == NULL) {  // 内核线程
        ASSERT((uint32_t)ptr >= KERNEL_HEAP_START);
        pf = PF_KERNEL;
        p = &kernel_pool;
    } else {  // 用户进程
        pf = PF_USER;
        p = &user_pool;
    }

    lock_acquire(&p->lock);
    struct mem_block *b = ptr;
    struct arena *a = block2arena(b);
    ASSERT(a->large == 0 || a->large == 1);
    if (a->desc == NULL && a->large == true) {  // large
        mfree_page(pf, a, a->cnt);
    } else {  // not large
        list_append(&a->desc->free_list, &b->free_node);
        if (++a->cnt == a->desc->blocks_per_arena) {
            for (int i = 0; i < a->desc->blocks_per_arena; i++) {
                struct mem_block *b = arena2block(a, i);  // i = block index
                ASSERT(node_find(&a->desc->free_list, &b->free_node));
                list_remove(&b->free_node);
            }
            mfree_page(pf, a, 1);
        }
    }
    lock_release(&p->lock);
}

/**
 * @brief 初始化内存池
 * 
 */
void mem_pool_init(uint32_t all_mem) {
    //put_str("\nmemory pool init start\n", 0x07);
    uint32_t page_table_size = 256 * PG_SIZE;        // 页目录表 +　页表　共占用的空间
    uint32_t mem_used = page_table_size + 0x100000;  // 　页目录表＋页表＋低端１MB
    uint32_t mem_free = all_mem - mem_used;
    uint16_t free_page_all = mem_free / PG_SIZE;
    uint16_t free_page_kernel = free_page_all / 2;  // 物理内存池划分为两个等大的内存池
    uint16_t free_page_user = free_page_all - free_page_kernel;

    uint32_t bitmap_len_kernel = free_page_kernel / 8;  // bitmap中1位表示1页
    uint32_t bitmap_len_user = free_page_user / 8;

    uint32_t kernel_pool_start = mem_used;
    uint32_t user_pool_start = kernel_pool_start + free_page_kernel * PG_SIZE;

    // 内核内存池
    kernel_pool.phy_addr_start = kernel_pool_start;
    kernel_pool.pool_size = free_page_kernel * PG_SIZE;
    kernel_pool.pool_bitmap.len = bitmap_len_kernel;
    kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
    bitmap_init(&kernel_pool.pool_bitmap);
    lock_init(&kernel_pool.lock);

    //用户内存池
    user_pool.phy_addr_start = user_pool_start;
    user_pool.pool_size = free_page_user * PG_SIZE;
    user_pool.pool_bitmap.len = bitmap_len_user;
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + bitmap_len_kernel);
    bitmap_init(&user_pool.pool_bitmap);
    lock_init(&user_pool.lock);

    //put_str("      kernel_pool_bitmap_start:", 0x07);
    //put_uint((int)kernel_pool.pool_bitmap.bits, 16, 0x07);
    //put_str(" kernel_pool_phy_addr_start:", 0x07);
    //put_int(kernel_pool.phy_addr_start, 16, 0x07);
    //put_str("\n", 0x07);
    //put_str("      user_pool_bitmap_start:", 0x07);
    //put_uint((int)user_pool.pool_bitmap.bits, 16, 0x07);
    //put_str(" user_pool_phy_addr_start:", 0x07);
    //put_int(user_pool.phy_addr_start, 16, 0x07);
    //put_str("\n", 0x07);

    // 内核虚拟地址(堆)
    kernel_vaddr.vaddr_bitmap.len = bitmap_len_kernel;
    kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_BASE + bitmap_len_kernel + bitmap_len_user);
    kernel_vaddr.vaddr_start = KERNEL_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);

    //put_uint(MEM_BITMAP_BASE, 16, 0x07);
    //put_char('\n', 0x07);
    //put_uint(MEM_BITMAP_BASE, 10, 0x07);
    //put_str("kernel_vaddr.vaddr_bitmap.start:", 0x07);
    //put_uint((int)kernel_vaddr.vaddr_bitmap.bits, 16, 0x07);
    //put_str("\n", 0x07);
    //put_str("kernel_vaddr.vaddr_start:", 0x07);
    //put_uint((int)kernel_vaddr.vaddr_start, 16, 0x07);
    //put_str("\n", 0x07);

    //put_str("\nmem pool init done\n", 0x07);
}

/**
 * @brief 初始化内存块描述符
 * 
 * @param descs 
 */
void block_desc_init(struct mem_block_desc *descs) {
    uint16_t block_size = 16;
    for (int i = 0; i < DESC_CNT; i++) {
        descs[i].block_size = block_size;
        descs[i].blocks_per_arena = (PG_SIZE - sizeof(struct arena)) / block_size;
        list_init(&descs[i].free_list);
        block_size *= 2;
    }
}

/**
 * @brief 初始化内存
 * 
 */
void mem_init() {
    mem_pool_init(0x2000000);  // 32MB
    block_desc_init(kernel_block_descs);
}
