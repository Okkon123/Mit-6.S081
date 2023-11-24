// // Physical memory allocator, for user processes,
// // kernel stacks, page-table pages,
// // and pipe buffers. Allocates whole 4096-byte pages.

// #include "types.h"
// #include "param.h"
// #include "memlayout.h"
// #include "spinlock.h"
// #include "riscv.h"
// #include "defs.h"

// void freerange(void *pa_start, void *pa_end);

// extern char end[]; // first address after kernel.
//                    // defined by kernel.ld.

// struct run {
//   struct run *next;
// };

// struct {
//   struct spinlock lock;
//   struct run *freelist;
// } kmem;

// void
// kinit()
// {
//   initlock(&kmem.lock, "kmem");
//   freerange(end, (void*)PHYSTOP);
// }

// void
// freerange(void *pa_start, void *pa_end)
// {
//   char *p;
//   p = (char*)PGROUNDUP((uint64)pa_start);
//   for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
//     kfree(p);
// }

// // Free the page of physical memory pointed at by v,
// // which normally should have been returned by a
// // call to kalloc().  (The exception is when
// // initializing the allocator; see kinit above.)
// void
// kfree(void *pa)
// {
//   struct run *r;

//   if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
//     panic("kfree");

//   // Fill with junk to catch dangling refs.
//   memset(pa, 1, PGSIZE);

//   r = (struct run*)pa;

//   acquire(&kmem.lock);
//   r->next = kmem.freelist;
//   kmem.freelist = r;
//   release(&kmem.lock);
// }

// // Allocate one 4096-byte page of physical memory.
// // Returns a pointer that the kernel can use.
// // Returns 0 if the memory cannot be allocated.
// void *
// kalloc(void)
// {
//   struct run *r;

//   acquire(&kmem.lock);
//   r = kmem.freelist;
//   if(r)
//     kmem.freelist = r->next;
//   release(&kmem.lock);

//   if(r)
//     memset((char*)r, 5, PGSIZE); // fill with junk
//   return (void*)r;
// }


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
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    initlock(&kmem[i].lock, "kmem");
  }
  // uint64 tot_start = (uint64) PGROUNDUP((uint64) end);
  // uint64 tot_end = (uint64) PHYSTOP;
  // uint64 pa_start = tot_start + (tot_end - tot_start) * curCpuId / NCPU;
  // uint64 pa_end = tot_start + (tot_end - tot_start) * (curCpuId + 1) / NCPU;
  freerange((void*) end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*) pa_end; p += PGSIZE) {
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP) {
    panic("kfree");
  }
    
  uint64 tot_start = (uint64) PGROUNDUP((uint64) end);
  uint64 tot_end = (uint64) PHYSTOP;
  uint64 block = (tot_end - tot_start)/NCPU;
  int targetCpu = (int)(((uint64)pa - tot_start) / block);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[targetCpu].lock);
  r->next = kmem[targetCpu].freelist;
  kmem[targetCpu].freelist = r;
  release(&kmem[targetCpu].lock);

  // memset(pa, 1, PGSIZE);
  // r = (struct run*)pa;
  // push_off();  
  // int id = cpuid();
  // acquire(&kmem[id].lock);
  // r->next = kmem[id].freelist;
  // kmem[id].freelist = r;
  // release(&kmem[id].lock);
  // pop_off();  
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int curCpuId = cpuid();
  acquire(&kmem[curCpuId].lock);
  r = kmem[curCpuId].freelist;
  if(r) {
    kmem[curCpuId].freelist = r->next;
  } else {
    for (int i = 0; i < NCPU; i++) {
      if (i == curCpuId) {
        continue;
      }
      acquire(&kmem[i].lock);
      r = kmem[i].freelist;
      if(r) {
        kmem[i].freelist = r->next;
        release(&kmem[i].lock);
        break;
      }
      release(&kmem[i].lock);
    }
  }
  release(&kmem[curCpuId].lock);
  pop_off();
  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
  } 

  return (void*)r;
}