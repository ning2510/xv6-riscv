struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint timestamp;
  uint refcnt;      // 每个块缓冲区的引用计数
  struct buf *next;
  uchar data[BSIZE];
};

