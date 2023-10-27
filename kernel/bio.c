// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
} bcache;
void
printcore()
{
  push_off();
  int cpu = cpuid();
  pop_off();
  printf("cpu: %d ", cpu);
}
void
binit(void)
{
  initlock(&bcache.lock, "bcache");
  for(int i = 0; i < NBUF; i++){
    initlock(&bcache.buf[i].reflock, "bcache.bucket");
    initsleeplock(&bcache.buf[i].lock, "buffer");
  }
}

uint
hash(uint dev, uint blockno)
{
  uint result = (dev + blockno) % NBUF;
  return result;
}
// uint
// find(uint dev, uint blockno)
// {
//   uint i, idx, result;
//   idx = hash(dev, blockno);
//   for(i = 0; i < NBUF; i++){
//     acquire(&bcache.buf[idx].reflock);
//     release(&bcache.buf[idx].reflock);
    
//   }
//   return result;
// }
// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // Is the block already cached?
  uint i, idx;
  idx = hash(dev, blockno);

  // printcore();
  // printf("hash: %d ", idx);
  for(i = 0; i < NBUF; i++){
    b = bcache.buf + idx;

    acquire(&b->reflock);
    if(b->dev == dev && b->blockno == blockno){
      // printf("bget cached: %d\n", i);
      b->refcnt++;

      acquire(&tickslock);
      b->tick = ticks;
      release(&tickslock);

      release(&b->reflock);
      acquiresleep(&b->lock);
      return b;
    }
    release(&b->reflock);
    idx = (idx + 1) % NBUF;
  }
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  
  int lst = -1;
  int lstidx = 0;

  for(i = 0; i < NBUF; i++){
    b = bcache.buf + idx;

    acquire(&b->reflock);

    if((b->refcnt == 0) && (b->tick == 0)){
      // printf("bget not cached: %d\n", i);
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;

      acquire(&tickslock);
      b->tick = ticks;
      release(&tickslock);

      release(&b->reflock);

      acquiresleep(&b->lock);
      return b;
    }

    if((b->refcnt == 0) && (b->tick <= (uint)lst)){
      lst = b->tick;
      lstidx = idx;
    }
    release(&b->reflock);
    idx = (idx + 1) % NBUF;
  }
  if(lst < 0){
    panic("bget: no buffers");
  }
  // printf("bget not cached lst: %d\n", lstidx);
  b = bcache.buf + lstidx;

  acquire(&b->reflock);
  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;

  acquire(&tickslock);
  b->tick = ticks;
  release(&tickslock);
  
  release(&b->reflock);
  acquiresleep(&b->lock);
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&b->reflock);
  b->refcnt--;
  if(b->refcnt == 0){
    acquire(&tickslock);
    b->tick = ticks;
    release(&tickslock);
  }
  release(&b->reflock);
  
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


