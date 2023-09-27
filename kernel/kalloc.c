// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int refer[32800];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

static uint64 pastart;
uint64 pagenum;
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  pastart = (uint64)p;

  pagenum = 0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    krefer((uint64)p, 0);
    krefer((uint64)p, 1);
    kfree(p);
  }
  printf("nm:%d\n", pagenum);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  
  acquire(&kmem.lock);
  kmem.refer[REFERINDEX(pa)] -= 1;
  if(kmem.refer[REFERINDEX(pa)] > 10 || kmem.refer[REFERINDEX(pa)] < 0){
    printf("very big or small\n");
  }
  if(kmem.refer[REFERINDEX(pa)] == 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    r->next = kmem.freelist;
    kmem.freelist = r;
    pagenum++;
  }
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  if(pagenum == 0){
    return 0;
  }
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(kmem.refer[REFERINDEX(r)] != 0){
    printf("kalloc: refer\n");
  }
  if(r){
    kmem.freelist = r->next;
    kmem.refer[REFERINDEX(r)] = 1;
  }
  pagenum--;
  release(&kmem.lock);
  if(pagenum % 5000 == 0){
    // printf("pgnum: %d\n", pagenum);
  }
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Adjust the reference.
int
krefer(uint64 pa, int op)
{
  acquire(&kmem.lock);
  if(op == 0){
    kmem.refer[REFERINDEX(pa)] = 0;
  } else{
    kmem.refer[REFERINDEX(pa)] += op;
  }
  release(&kmem.lock);
  return 0;
}