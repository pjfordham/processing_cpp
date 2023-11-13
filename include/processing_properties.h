#ifndef PROCESSING_PROPERTIES_H
#define PROCESSING_PROPERTIES_H

#include <fmt/core.h>
#include "processing_math.h"


template <typename T, typename V, V& (T::*first)(), const V& (T::*cfirst)() const, std::size_t (*offset)() >
class property_t {
   constexpr const T &cref() const {
      return *(reinterpret_cast<const T *>(this) - offset());
   }
   constexpr T &ref() {
      return *(reinterpret_cast<T *>(this) - offset());
   }
public:
   constexpr operator V& () {
      return (ref().*first)();
   }
   constexpr operator const V& () const {
      return (cref().*cfirst)();
   }
   constexpr V &operator=(V a) {
      return (ref().*first)() = a;
   }
};

template <typename T, typename V, V& (T::*first)(), const V& (T::*cfirst)() const, std::size_t (*offset)() >
struct fmt::formatter<property_t<T,V,first,cfirst,offset>> {
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }
   template <typename FormatContext>
   auto format( const property_t<T,V,first,cfirst,offset>& v, FormatContext& ctx) {
      V s = v;
      return fmt::format_to(ctx.out(), "{}",s);
   }
};

#endif
