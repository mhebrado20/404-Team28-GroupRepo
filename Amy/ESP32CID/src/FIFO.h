#ifndef UTIL_FIFO_H
#define UTIL_FIFO_H

/*
    FIFO.h

    Adapted from LibAPRS by Mark Qvist https://github.com/markqvist  http://unsigned.io/

    - added maxQuLen parameter for end of run stats.

    Ver: ESP8266 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) not supported directly.
*/


#include <stddef.h>
// #include <util/atomic.h>

typedef struct FIFOBuffer
{
  unsigned char *begin;
  unsigned char *end;
  unsigned char * volatile head;
  unsigned char * volatile tail;
  volatile int maxQuLen ;
} FIFOBuffer;

inline bool fifo_isempty(const FIFOBuffer *f) {
  return f->head == f->tail;
}

inline bool fifo_isfull(const FIFOBuffer *f) {
  return ((f->head == f->begin) && (f->tail == f->end)) || (f->tail == f->head - 1);
}

inline void fifo_push(FIFOBuffer *f, unsigned char c) {
  *(f->tail) = c;

  if (f->tail == f->end) {
    f->tail = f->begin;
  } else {
    f->tail++;
  }

  int qItems ;
  if ( f->tail >= f->head ) {
    qItems = f->tail - f->head ;
  }
  else qItems = 1 +  ( f->end - f->head ) +  (f->tail - f->begin ) ;
  if ( qItems > f->maxQuLen ) f->maxQuLen = qItems ;

}

inline unsigned char fifo_pop(FIFOBuffer *f) {
  if (f->head == f->end) {
    f->head = f->begin;
    return *(f->end);
  } else {
    return *(f->head++);
  }
}

inline void fifo_flush(FIFOBuffer *f) {
  // f->head = f->tail;    // f->head = f->tail = f->begin  ???  te_24
  f->head = f->tail = f->begin ;
  f->maxQuLen = 0 ;
}

inline bool fifo_isempty_locked(const FIFOBuffer *f) {
  bool result;
  //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  result = fifo_isempty(f);
  //}
  return result;
}

inline bool fifo_isfull_locked(const FIFOBuffer *f) {
  bool result;
  //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  result = fifo_isfull(f);
  //}
  return result;
}

inline void fifo_push_locked(FIFOBuffer *f, unsigned char c) {
  //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  fifo_push(f, c);
  //}
}

inline unsigned char fifo_pop_locked(FIFOBuffer *f) {
  unsigned char c;
  //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  c = fifo_pop(f);
  //}
  return c;
}

inline void fifo_init(FIFOBuffer *f, unsigned char *buffer, size_t size) {
  f->head = f->tail = f->begin = buffer;
  f->end = buffer + size - 1;
  f->maxQuLen = 0 ;
}

inline size_t fifo_len(FIFOBuffer *f) {
  return f->end - f->begin;
}

inline int fifo_maxQuLen(FIFOBuffer *f) {
  return f->maxQuLen ;
}


inline void fifo_maxQuLenReset(FIFOBuffer *f) {
  f->maxQuLen = 0 ;
}

// te_44 te_45
inline void fifo_maxQuLenReset_locked(FIFOBuffer *f) {
  //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  f->maxQuLen = 0 ;
  //}
}


#endif





















