#pragma once
#include <vector>
#include <cstring>
struct queue_t { std::vector<unsigned char> bytes; unsigned item=0, capacity=0, head=0, count=0; };
inline void queue_init(queue_t *q, unsigned item, unsigned count) {
    q->item=item; q->capacity=count; q->head=q->count=0; q->bytes.assign(item*count,0);
}
inline bool queue_try_add(queue_t *q, const void *p) {
    if(q->count==q->capacity) return false;
    std::memcpy(q->bytes.data()+((q->head+q->count)%q->capacity)*q->item,p,q->item);
    ++q->count; return true;
}
inline bool queue_try_remove(queue_t *q, void *p) {
    if(!q->count) return false;
    std::memcpy(p,q->bytes.data()+q->head*q->item,q->item);
    q->head=(q->head+1)%q->capacity; --q->count; return true;
}
inline unsigned queue_get_level(queue_t *q) { return q->count; }
