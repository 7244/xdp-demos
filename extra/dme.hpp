#pragma once

#include <type_traits>
#include <functional>

#ifndef STR
  #define STR(_m) #_m
#endif
#ifndef CONCAT
  #define CONCAT(_0_m, _1_m) _0_m ## _1_m
#endif
#ifndef __empty_struct
  #define __empty_struct __empty_struct
  typedef struct{
  }__empty_struct;
#endif
#ifndef __ofof
  #define __ofof __ofof
  #pragma pack(push, 1)
    template <typename Member, std::size_t O>
    struct __Pad_t {
      char pad[O];
      Member m;
    };
  #pragma pack(pop)

  template<typename Member>
  struct __Pad_t<Member, 0> {
    Member m;
  };

  template <typename Base, typename Member, std::size_t O>
  struct __MakeUnion_t {
    union U {
      char c;
      Base base;
      __Pad_t<Member, O> pad;
      constexpr U() noexcept : c{} {};
    };
    constexpr static U u{};
  };

  template <typename Member, typename Base, typename Orig>
  struct __ofof_impl {
    template<std::size_t off, auto union_part = &__MakeUnion_t<Base, Member, off>::u>
    static constexpr std::ptrdiff_t offset2(Member Orig::* member) {
      if constexpr (off > sizeof(Base)) {
        throw 1;
      }
      else {
        const auto diff1 = &((static_cast<const Orig*>(&union_part->base))->*member);
        const auto diff2 = &union_part->pad.m;
        if (diff1 > diff2) {
          constexpr auto MIN = sizeof(Member) < alignof(Orig) ? sizeof(Member) : alignof(Orig);
          return offset2<off + MIN>(member);
        }
        else {
          return off;
        }
      }
    }
  };

  template<class Member, class Base>
  std::tuple<Member, Base> __get_types(Member Base::*);

  template <class TheBase = void, class TT>
  inline constexpr std::ptrdiff_t __ofof(TT member) {
    using T = decltype(__get_types(std::declval<TT>()));
    using Member = std::tuple_element_t<0, T>;
    using Orig = std::tuple_element_t<1, T>;
    using Base = std::conditional_t<std::is_void_v<TheBase>, Orig, TheBase>;
    return __ofof_impl<Member, Base, Orig>::template offset2<0>(member);
  }

  template <auto member, class TheBase = void>
  inline constexpr std::ptrdiff_t __ofof() {
    return __ofof<TheBase>(member);
  }
#endif
#ifndef __return_type_of
  #define __return_type_of __return_type_of
  template <typename Callable>
  using __return_type_of = typename decltype(std::function{ std::declval<Callable>() })::result_type;
#endif
#ifndef __compiletime_str
  #define __compiletime_str __compiletime_str
  template<std::size_t count>
  struct __compiletime_str
  {
    char buffer[count + 1]{};
    int length = count;

    constexpr __compiletime_str(char const* string)
    {
      for (std::size_t i = 0; i < count; ++i) {
        buffer[i] = string[i];
      }
    }
    constexpr operator char const* () const { return buffer; }
  };
  template<std::size_t count>
  __compiletime_str(char const (&)[count])->__compiletime_str<count - 1>;
#endif
#ifndef __conditional_value
  #define __conditional_value __conditional_value
  template <bool _Test, uintptr_t _Ty1, uintptr_t _Ty2>
  struct __conditional_value {
    static constexpr auto value = _Ty1;
  };
  template <uintptr_t _Ty1, uintptr_t _Ty2>
  struct __conditional_value<false, _Ty1, _Ty2> {
    static constexpr auto value = _Ty2;
  };
  template <bool _Test, uintptr_t _Ty1, uintptr_t _Ty2>
  struct __conditional_value_t {
    static constexpr auto value = __conditional_value<_Test, _Ty1, _Ty2>::value;
  };
#endif
#ifndef __dme
  template <__compiletime_str StringName = "", typename type = __empty_struct, typename CommonData_t = __empty_struct>
  struct __dme_t : CommonData_t{
    const char *sn = StringName;
    // data type
    using dt = type;
    /* data struct size */
    uint32_t m_DSS = __conditional_value<
      std::is_empty<dt>::value,
      0,
      sizeof(dt)
    >::value;
  };
  #define __dme(p0, p1) \
    using CONCAT(p0,_t) = __dme_t<STR(p0), __return_type_of<decltype([] { \
      struct{p1} v; \
      return v; \
    })>, CommonData>; \
    CONCAT(p0,_t) p0
  template <typename inherited_t, typename CommonData_t = __empty_struct>
  struct __dme_inherit{
    using CommonData = CommonData_t;
    using _dme_type = __dme_t<"", __empty_struct, CommonData>;

    static constexpr uint32_t GetMemberAmount(){
      return sizeof(inherited_t) / sizeof(_dme_type);
    }

    /* number to address */
    _dme_type *NA(uintptr_t CI){
      return (_dme_type *)((uint8_t *)this + CI * sizeof(_dme_type));
    }

    /* address to number */
    static constexpr uintptr_t AN(auto inherited_t:: *C){
      return __ofof(C) / sizeof(_dme_type);
    }

    /* is number invalid */
    bool ICI(uintptr_t CI){
      return CI * sizeof(_dme_type) > sizeof(inherited_t);
    }
  };
#endif
