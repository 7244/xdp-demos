#include <type_traits>
#include <functional>

#ifndef __empty_struct
  #define __empty_struct __empty_struct
  typedef struct{
  }__empty_struct;
#endif
#ifndef CONCAT
  #define CONCAT(_0_m, _1_m) _0_m ## _1_m
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
  template <typename type = __empty_struct, typename CommonData_t = __empty_struct>
  struct __dme_t : CommonData_t{
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
    using CONCAT(p0,_t) = __dme_t<__return_type_of<decltype([] { \
      struct{p1} v; \
      return v; \
    })>, CommonData>; \
    CONCAT(p0,_t) p0
  template <typename inherited_t, typename CommonData_t = __empty_struct>
  struct __dme_inherit{
    using CommonData = CommonData_t;
    using _dme_type = __dme_t<__empty_struct, CommonData>;

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

struct country_code_data_t{
  const char *alpha2;
};

#define d(p_full_name, p_alpha2) \
  __dme(p_full_name,) = {{.alpha2 = #p_alpha2}}

struct country_code_t : __dme_inherit<country_code_t, country_code_data_t>{
  d(ARIPO, AP);
  d(European_Union, EU);

  d(Kosovo, XK);
  d(Andorra, AD);
  d(United_Arab_Emirates, AE);
  d(Afghanistan, AF);
  d(Antigua_and_Barbuda, AG);
  d(Anguilla, AI);
  d(Albania, AL);
  d(Armenia, AM);
  d(Angola, AO);
  d(Antarctica, AQ);
  d(Argentina, AR);
  d(American_Samoa, AS);
  d(Austria, AT);
  d(Australia, AU);
  d(Aruba, AW);
  d(Åland_Islands, AX);
  d(Azerbaijan, AZ);
  d(Bosnia_and_Herzegovina, BA);
  d(Barbados, BB);
  d(Bangladesh, BD);
  d(Belgium, BE);
  d(Burkina_Faso, BF);
  d(Bulgaria, BG);
  d(Bahrain, BH);
  d(Burundi, BI);
  d(Benin, BJ);
  d(Saint_Barthélemy, BL);
  d(Bermuda, BM);
  d(Brunei_Darussalam, BN);
  d(Bolivia, BO);
  d(Bonaire_Sint_Eustatius_and_Saba, BQ);
  d(Brazil, BR);
  d(Bahamas, BS);
  d(Bhutan, BT);
  d(Bouvet_Island, BV);
  d(Botswana, BW);
  d(Belarus, BY);
  d(Belize, BZ);
  d(Canada, CA);
  d(Cocos, CC);
  d(Democratic_Republic_of_the_Congo, CD);
  d(Central_African_Republic, CF);
  d(Congo, CG);
  d(Switzerland, CH);
  d(Côte_dIvoire, CI);
  d(Cook_Islands, CK);
  d(Chile, CL);
  d(Cameroon, CM);
  d(China, CN);
  d(Colombia, CO);
  d(Costa_Rica, CR);
  d(Cuba, CU);
  d(Cabo_Verde, CV);
  d(Curaçao, CW);
  d(Christmas_Island, CX);
  d(Cyprus, CY);
  d(Czechia, CZ);
  d(Germany, DE);
  d(Djibouti, DJ);
  d(Denmark, DK);
  d(Dominica, DM);
  d(Dominican_Republic, DO);
  d(Algeria, DZ);
  d(Ecuador, EC);
  d(Estonia, EE);
  d(Egypt, EG);
  d(Western_Sahara, EH);
  d(Eritrea, ER);
  d(Spain, ES);
  d(Ethiopia, ET);
  d(Finland, FI);
  d(Fiji, FJ);
  d(Falkland_Islands, FK);
  d(Micronesia, FM);
  d(Faroe_Islands, FO);
  d(France, FR);
  d(Gabon, GA);
  d(United_Kingdom_of_Great_Britain_and_Northern_Ireland, GB);
  d(Grenada, GD);
  d(Georgia, GE);
  d(French_Guiana, GF);
  d(Guernsey, GG);
  d(Ghana, GH);
  d(Gibraltar, GI);
  d(Greenland, GL);
  d(Gambia, GM);
  d(Guinea, GN);
  d(Guadeloupe, GP);
  d(Equatorial_Guinea, GQ);
  d(Greece, GR);
  d(South_Georgia_and_the_South_Sandwich_Islands, GS);
  d(Guatemala, GT);
  d(Guam, GU);
  d(Guinea_Bissau, GW);
  d(Guyana, GY);
  d(Hong_Kong, HK);
  d(Heard_Island_and_McDonald_Islands, HM);
  d(Honduras, HN);
  d(Croatia, HR);
  d(Haiti, HT);
  d(Hungary, HU);
  d(Indonesia, ID);
  d(Ireland, IE);
  d(Israel, IL);
  d(Isle_of_Man, IM);
  d(India, IN);
  d(British_Indian_Ocean_Territory, IO);
  d(Iraq, IQ);
  d(Iran, IR);
  d(Iceland, IS);
  d(Italy, IT);
  d(Jersey, JE);
  d(Jamaica, JM);
  d(Jordan, JO);
  d(Japan, JP);
  d(Kenya, KE);
  d(Kyrgyzstan, KG);
  d(Cambodia, KH);
  d(Kiribati, KI);
  d(Comoros, KM);
  d(Saint_Kitts_and_Nevis, KN);
  d(North_Korea, KP);
  d(South_Korea, KR);
  d(Kuwait, KW);
  d(Cayman_Islands, KY);
  d(Kazakhstan, KZ);
  d(Laos, LA);
  d(Lebanon, LB);
  d(Saint_Lucia, LC);
  d(Liechtenstein, LI);
  d(Sri_Lanka, LK);
  d(Liberia, LR);
  d(Lesotho, LS);
  d(Lithuania, LT);
  d(Luxembourg, LU);
  d(Latvia, LV);
  d(Libya, LY);
  d(Morocco, MA);
  d(Monaco, MC);
  d(Moldova, MD);
  d(Montenegro, ME);
  d(Saint_Martin, MF);
  d(Madagascar, MG);
  d(Marshall_Islands, MH);
  d(Republic_of_North_Macedonia, MK);
  d(Mali, ML);
  d(Myanmar, MM);
  d(Mongolia, MN);
  d(Macao, MO);
  d(Northern_Mariana_Islands, MP);
  d(Martinique, MQ);
  d(Mauritania, MR);
  d(Montserrat, MS);
  d(Malta, MT);
  d(Mauritius, MU);
  d(Maldives, MV);
  d(Malawi, MW);
  d(Mexico, MX);
  d(Malaysia, MY);
  d(Mozambique, MZ);
  d(Namibia, NA);
  d(New_Caledonia, NC);
  d(Niger, NE);
  d(Norfolk_Island, NF);
  d(Nigeria, NG);
  d(Nicaragua, NI);
  d(Netherlands, NL);
  d(Norway, NO);
  d(Nepal, NP);
  d(Nauru, NR);
  d(Niue, NU);
  d(New_Zealand, NZ);
  d(Oman, OM);
  d(Panama, PA);
  d(Peru, PE);
  d(French_Polynesia, PF);
  d(Papua_New_Guinea, PG);
  d(Philippines, PH);
  d(Pakistan, PK);
  d(Poland, PL);
  d(Saint_Pierre_and_Miquelon, PM);
  d(Pitcairn, PN);
  d(Puerto_Rico, PR);
  d(Palestine_State_of, PS);
  d(Portugal, PT);
  d(Palau, PW);
  d(Paraguay, PY);
  d(Qatar, QA);
  d(Réunion, RE);
  d(Romania, RO);
  d(Serbia, RS);
  d(Russian_Federation, RU);
  d(Rwanda, RW);
  d(Saudi_Arabia, SA);
  d(Solomon_Islands, SB);
  d(Seychelles, SC);
  d(Sudan, SD);
  d(Sweden, SE);
  d(Singapore, SG);
  d(Saint_Helena_Ascension_and_Tristan_da_Cunha, SH);
  d(Slovenia, SI);
  d(Svalbard_and_Jan_Mayen, SJ);
  d(Slovakia, SK);
  d(Sierra_Leone, SL);
  d(San_Marino, SM);
  d(Senegal, SN);
  d(Somalia, SO);
  d(Suriname, SR);
  d(South_Sudan, SS);
  d(Sao_Tome_and_Principe, ST);
  d(El_Salvador, SV);
  d(Sint_Maarten, SX);
  d(Syrian_Arab_Republic, SY);
  d(Eswatini, SZ);
  d(Turks_and_Caicos_Islands, TC);
  d(Chad, TD);
  d(French_Southern_Territories, TF);
  d(Togo, TG);
  d(Thailand, TH);
  d(Tajikistan, TJ);
  d(Tokelau, TK);
  d(Timor_Leste, TL);
  d(Turkmenistan, TM);
  d(Tunisia, TN);
  d(Tonga, TO);
  d(Turkey, TR);
  d(Trinidad_and_Tobago, TT);
  d(Tuvalu, TV);
  d(Taiwan, TW);
  d(Tanzania_United_Republic_of, TZ);
  d(Ukraine, UA);
  d(Uganda, UG);
  d(United_States_Minor_Outlying_Islands, UM);
  d(United_States_of_America, US);
  d(Uruguay, UY);
  d(Uzbekistan, UZ);
  d(Holy_See, VA);
  d(Saint_Vincent_and_the_Grenadines, VC);
  d(Venezuela, VE);
  d(Virgin_Islands_British, VG);
  d(Virgin_Islands_US, VI);
  d(Viet_Nam, VN);
  d(Vanuatu, VU);
  d(Wallis_and_Futuna, WF);
  d(Samoa, WS);
  d(Yemen, YE);
  d(Mayotte, YT);
  d(South_Africa, ZA);
  d(Zambia, ZM);
  d(Zimbabwe, ZW);
  d(Unknown, Unknown);
}country_code;

#undef d
