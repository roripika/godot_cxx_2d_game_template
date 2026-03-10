#include <cstddef>
#include <functional>

// We need to match the internal libc++ declaration exactly.
// Including <__config> (internal header) or <string> usually brings in the
// necessary macros.
#include <__config>

namespace std {
inline namespace __1 {

// Use the same macros as the declaration in <__functional/hash.h>
size_t __hash_memory(_LIBCPP_NOESCAPE const void *__p, size_t __n) _NOEXCEPT {
  if (__n == 0)
    return 0;

  // Very simple FNV-1a like hash for the memory block
  size_t __h = 2166136261u;
  const unsigned char *__curr = static_cast<const unsigned char *>(__p);
  const unsigned char *__end = __curr + __n;
  while (__curr < __end) {
    __h ^= static_cast<size_t>(*__curr++);
    __h *= 16777619u;
  }
  return __h;
}

} // namespace __1
} // namespace std
