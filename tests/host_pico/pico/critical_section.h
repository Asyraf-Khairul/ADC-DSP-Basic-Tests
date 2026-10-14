#pragma once
struct critical_section_t { void *spin_lock=nullptr; };
inline void spin_lock_unsafe_blocking(void*) {}
inline void spin_unlock_unsafe(void*) {}
inline void critical_section_init_with_lock_num(critical_section_t*, unsigned) {}
inline void critical_section_enter_blocking(critical_section_t*) {}
inline void critical_section_exit(critical_section_t*) {}
