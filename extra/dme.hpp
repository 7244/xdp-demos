#pragma once

#ifndef __empty_struct
  #define __empty_struct __empty_struct
  typedef struct{
  }__empty_struct;
#endif
#ifndef __compiletime_str
  #define __compiletime_str __compiletime_str
  template<uintptr_t count>
  struct __compiletime_str
  {
    char buffer[count + 1]{};
    uintptr_t length = count;

    constexpr __compiletime_str(char const* string){
      for (uintptr_t i = 0; i < count; ++i) {
        buffer[i] = string[i];
      }
    }
    constexpr operator char const* () const { return buffer; }
  };
  template<uintptr_t count>
  __compiletime_str(char const (&)[count])->__compiletime_str<count - 1>;
#endif
#ifndef __dme
  template <typename T, uintptr_t idx, __compiletime_str str = "", typename dt_t = void>
  struct __dme_t : T {
    using dt = dt_t;
    static constexpr uintptr_t I = idx;
    constexpr operator uintptr_t() { return I; }
    static constexpr unsigned long long AN() {
      return I;
    }
    const char* sn = str;
  };

  // hardcoded to only lambdas
  template<typename T>
  using return_type_of_t = decltype((*(T*)nullptr)());


  #define __dme(varname, data) __dme_t<dme_type_t, __COUNTER__ - DME_INTERNAL__BEG - 1, #varname, \
    return_type_of_t<decltype([]{ \
      struct { \
        data \
      }v; \
      return v;  \
    })> \
  > varname

  template <typename main_t, uintptr_t index, typename T = __empty_struct>
  struct __dme_inherit_t {
    using dme_type_t = T;

    constexpr __dme_t<dme_type_t, 0, ""> * NA(unsigned long long I) const {
      return &((__dme_t<dme_type_t, 0, ""> *)this)[I];
    }

    static constexpr unsigned long long GetMemberAmount() {
      return sizeof(main_t) / sizeof(dme_type_t);
    }

    static constexpr auto DME_INTERNAL__BEG = index;
  };
  #define __dme_inherit(main_t, ...) __dme_inherit_t<main_t, __COUNTER__, ##__VA_ARGS__>
#endif
