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

extern uint ticks;

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
} bcache;

struct {
  struct spinlock lock;
  struct buf head;
} bucket[NBUCKET];

int hash(int key) {
  return key % NBUCKET;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  int avg = NBUF / NBUCKET;
  b = bcache.buf;
  for(int i = 0; i < NBUCKET; i++) {
    initlock(&bucket[i].lock, "bucket");
    for(int j = 0; j < avg; j++) {
      initsleeplock(&b->lock, "buffer");
      b->blockno = i;
      b->next = bucket[i].head.next;
      bucket[i].head.next = b;
      b++;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int idx = hash(blockno);
  acquire(&bucket[idx].lock);

  // Is the block already cached?
  for(b = bucket[idx].head.next; b; b = b->next) {
    if(b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bucket[idx].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  uint time = 1e9;
  struct buf *tbuf = 0;
  for(b = bucket[idx].head.next; b; b = b->next) {
    if(b->refcnt == 0 && b->timestamp < time) {
      time = b->timestamp;
      tbuf = b;
    }
  }

  if(tbuf)
    goto find;
  
  acquire(&bcache.lock);

refind:
  for(b = bcache.buf; b < bcache.buf + NBUF; b++) {
    if(b->refcnt == 0 && b->timestamp < time) {
      time = b->timestamp;
      tbuf = b;
    }
  }

  if(!tbuf)
    panic("bget: no buffers");

  int idx2 = hash(tbuf->blockno);
  acquire(&bucket[idx2].lock);
  if(tbuf->refcnt != 0) {
    release(&bucket[idx2].lock);
    time = 1e9;
    goto refind;
  }

  struct buf *cur = &bucket[idx2].head;
  while(cur->next != tbuf) {
    cur = cur->next;
  }

  // remove tbuf forom bucket[idx2]
  cur->next = tbuf->next;
  release(&bucket[idx2].lock);

  // add tbuf to bucket[idx]
  tbuf->next = bucket[idx].head.next;
  bucket[idx].head.next = tbuf;
  release(&bcache.lock);

find:
  tbuf->dev = dev;
  tbuf->blockno = blockno;
  tbuf->valid = 0;
  tbuf->refcnt = 1;
  release(&bucket[idx].lock);
  acquiresleep(&tbuf->lock);
  return tbuf;
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

  int idx = hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->timestamp = ticks;
  }
  
  release(&bucket[idx].lock);
}

void
bpin(struct buf *b) {
  int idx = hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt++;
  release(&bucket[idx].lock);
}

void
bunpin(struct buf *b) {
  int idx = hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt--;
  release(&bucket[idx].lock);
}


