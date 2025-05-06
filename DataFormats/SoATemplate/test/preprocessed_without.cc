#include <memory> /* clang -E -fkeep-system-includes */
#include <tuple>  /* clang -E -fkeep-system-includes */

#include <cstdint> /* clang -E -fkeep-system-includes */
#include <cassert> /* clang -E -fkeep-system-includes */
#include <cstring> /* clang -E -fkeep-system-includes */

#include <memory>                 /* clang -E -fkeep-system-includes */
#include <ostream>                /* clang -E -fkeep-system-includes */
#include <tuple>                  /* clang -E -fkeep-system-includes */
#include <type_traits>            /* clang -E -fkeep-system-includes */
#include <boost/preprocessor.hpp> /* clang -E -fkeep-system-includes */

typedef signed char cms_int8_t;
typedef unsigned char cms_uint8_t;
typedef short cms_int16_t;
typedef unsigned short cms_uint16_t;
typedef int cms_int32_t;
typedef unsigned int cms_uint32_t;
typedef long long cms_int64_t;
typedef unsigned long long cms_uint64_t;
namespace cms::soa {
  using size_type = cms_int32_t;
  using byte_size_type = std::size_t;
  enum class SoAColumnType { scalar = 0, column = 1, eigen = 2 };
  namespace RestrictQualify {
    constexpr bool enabled = true;
    constexpr bool disabled = false;
    constexpr bool Default = enabled;
  }  // namespace RestrictQualify
  namespace RangeChecking {
    constexpr bool enabled = true;
    constexpr bool disabled = false;
    constexpr bool Default = enabled;
  }  // namespace RangeChecking
  template <typename T, bool RESTRICT_QUALIFY>
  struct add_restrict {};
  template <typename T>
  struct add_restrict<T, RestrictQualify::enabled> {
    using Value = T;
    using Pointer = T* __restrict__;
    using Reference = T& __restrict__;
    using ConstValue = const T;
    using PointerToConst = const T* __restrict__;
    using ReferenceToConst = const T& __restrict__;
  };
  template <typename T>
  struct add_restrict<T, RestrictQualify::disabled> {
    using Value = T;
    using Pointer = T*;
    using Reference = T&;
    using ConstValue = const T;
    using PointerToConst = const T*;
    using ReferenceToConst = const T&;
  };
  template <SoAColumnType COLUMN_TYPE, typename T>
  struct SoAConstParametersImpl;
  template <SoAColumnType COLUMN_TYPE, typename T>
  struct SoAParametersImpl;
  template <SoAColumnType COLUMN_TYPE, typename T>
  struct SoAConstParametersImpl {
    static constexpr SoAColumnType columnType = COLUMN_TYPE;
    using ValueType = T;
    using ScalarType = T;
    using TupleOrPointerType = const ValueType*;
    SoAConstParametersImpl() = default;
    inline __attribute__((always_inline)) constexpr SoAConstParametersImpl(ValueType const* addr, size_type size)
        : addr_(addr), size_{size} {}
    inline __attribute__((always_inline)) constexpr SoAConstParametersImpl(
        SoAParametersImpl<columnType, ValueType> const& o)
        : addr_{o.addr_}, size_{o.size_} {}
    static constexpr bool checkAlignment(ValueType* addr, byte_size_type alignment) {
      return reinterpret_cast<intptr_t>(addr) % alignment;
    }
    TupleOrPointerType tupleOrPointer() { return addr_; }

  public:
    ValueType const* addr_ = nullptr;
    size_type size_ = 0;
  };
  template <typename T>
  struct SoAConstParametersImpl<SoAColumnType::eigen, T> {
    static constexpr SoAColumnType columnType = SoAColumnType::eigen;
    using ValueType = T;
    using ScalarType = typename T::Scalar;
    using TupleOrPointerType = std::tuple<ScalarType*, byte_size_type>;
    SoAConstParametersImpl() = default;
    inline __attribute__((always_inline)) constexpr SoAConstParametersImpl(ScalarType const* addr,
                                                                           byte_size_type stride,
                                                                           size_type size)
        : addr_(addr), stride_(stride), size_{size} {}
    inline __attribute__((always_inline)) constexpr SoAConstParametersImpl(TupleOrPointerType const& tuple)
        : addr_(std::get<0>(tuple)), stride_(std::get<1>(tuple)) {}
    inline __attribute__((always_inline)) constexpr SoAConstParametersImpl(
        SoAParametersImpl<columnType, ValueType> const& o)
        : addr_{o.addr_}, stride_{o.stride_}, size_{o.size_} {}
    static constexpr bool checkAlignment(TupleOrPointerType const& tuple, byte_size_type alignment) {
      const auto& [addr, stride] = tuple;
      return reinterpret_cast<intptr_t>(addr) % alignment;
    }
    TupleOrPointerType tupleOrPointer() { return {addr_, stride_}; }

  public:
    ScalarType const* addr_ = nullptr;
    byte_size_type stride_ = 0;
    size_type size_ = 0;
  };
  template <SoAColumnType COLUMN_TYPE>
  struct SoAConstParameters_ColumnType {
    template <typename T>
    using DataType = SoAConstParametersImpl<COLUMN_TYPE, T>;
  };
  template <SoAColumnType COLUMN_TYPE, typename T>
  struct SoAParametersImpl {
    static constexpr SoAColumnType columnType = COLUMN_TYPE;
    using ValueType = T;
    using ScalarType = T;
    using TupleOrPointerType = ValueType*;
    using ConstType = SoAConstParametersImpl<columnType, ValueType>;
    friend ConstType;
    SoAParametersImpl() = default;
    inline __attribute__((always_inline)) constexpr SoAParametersImpl(ValueType* addr, size_type size)
        : addr_(addr), size_{size} {}
    static constexpr bool checkAlignment(ValueType* addr, byte_size_type alignment) {
      return reinterpret_cast<intptr_t>(addr) % alignment;
    }
    TupleOrPointerType tupleOrPointer() { return addr_; }

  public:
    ValueType* addr_ = nullptr;
    size_type size_ = 0;
  };
  template <typename T>
  struct SoAParametersImpl<SoAColumnType::eigen, T> {
    static constexpr SoAColumnType columnType = SoAColumnType::eigen;
    using ValueType = T;
    using ScalarType = typename T::Scalar;
    using TupleOrPointerType = std::tuple<ScalarType*, byte_size_type>;
    using ConstType = SoAConstParametersImpl<columnType, ValueType>;
    friend ConstType;
    SoAParametersImpl() = default;
    inline __attribute__((always_inline)) constexpr SoAParametersImpl(ScalarType* addr,
                                                                      byte_size_type stride,
                                                                      size_type size)
        : addr_(addr), stride_(stride), size_(size) {}
    inline __attribute__((always_inline)) constexpr SoAParametersImpl(TupleOrPointerType const& tuple)
        : addr_(std::get<0>(tuple)), stride_(std::get<1>(tuple)) {}
    static constexpr bool checkAlignment(TupleOrPointerType const& tuple, byte_size_type alignment) {
      const auto& [addr, stride] = tuple;
      return reinterpret_cast<intptr_t>(addr) % alignment;
    }
    TupleOrPointerType tupleOrPointer() { return {addr_, stride_}; }

  public:
    ScalarType* addr_ = nullptr;
    byte_size_type stride_ = 0;
    size_type size_ = 0;
  };
  template <SoAColumnType COLUMN_TYPE>
  struct SoAParameters_ColumnType {
    template <typename T>
    using DataType = SoAParametersImpl<COLUMN_TYPE, T>;
  };
  namespace {
    template <typename T>
    constexpr inline std::remove_const_t<T>* non_const_ptr(T* p) {
      return const_cast<std::remove_const_t<T>*>(p);
    }
  }  // namespace
  template <SoAColumnType COLUMN_TYPE, typename T>
  inline __attribute__((always_inline)) constexpr SoAParametersImpl<COLUMN_TYPE, T> const_cast_SoAParametersImpl(
      SoAConstParametersImpl<COLUMN_TYPE, T> const& o) {
    return SoAParametersImpl<COLUMN_TYPE, T>{non_const_ptr(o.addr_), o.size_};
  }
  template <typename T>
  inline __attribute__((always_inline)) constexpr SoAParametersImpl<SoAColumnType::eigen, T>
  const_cast_SoAParametersImpl(SoAConstParametersImpl<SoAColumnType::eigen, T> const& o) {
    return SoAParametersImpl<SoAColumnType::eigen, T>{non_const_ptr(o.addr_), o.stride_, o.size_};
  }
  template <SoAColumnType COLUMN_TYPE,
            typename T,
            byte_size_type ALIGNMENT,
            bool RESTRICT_QUALIFY = RestrictQualify::disabled>
  class SoAValue {
    static_assert(COLUMN_TYPE != SoAColumnType::eigen);

  public:
    using Restr = add_restrict<T, RESTRICT_QUALIFY>;
    using Val = typename Restr::Value;
    using Ptr = typename Restr::Pointer;
    using Ref = typename Restr::Reference;
    using PtrToConst = typename Restr::PointerToConst;
    using RefToConst = typename Restr::ReferenceToConst;
    inline __attribute__((always_inline)) SoAValue(size_type i, T* col) : idx_(i), col_(col) {}
    inline __attribute__((always_inline)) SoAValue(size_type i, SoAParametersImpl<COLUMN_TYPE, T> params)
        : idx_(i), col_(params.addr_) {}
    inline __attribute__((always_inline)) Ref operator()() {
      Ptr col = col_;
      return col[idx_];
    }
    inline __attribute__((always_inline)) RefToConst operator()() const {
      PtrToConst col = col_;
      return col[idx_];
    }
    inline __attribute__((always_inline)) Ptr operator&() { return &col_[idx_]; }
    inline __attribute__((always_inline)) PtrToConst operator&() const { return &col_[idx_]; }
    using valueType = Val;
    static constexpr auto valueSize = sizeof(T);

  private:
    size_type idx_;
    T* col_;
  };
  template <class C, byte_size_type ALIGNMENT, bool RESTRICT_QUALIFY>
  class SoAValue<SoAColumnType::eigen, C, ALIGNMENT, RESTRICT_QUALIFY> {
    static_assert(!sizeof(C),
                  "Eigen/Core should be pre-included before the SoA headers to enable support for Eigen columns.");
  };
  template <SoAColumnType COLUMN_TYPE,
            typename T,
            byte_size_type ALIGNMENT,
            bool RESTRICT_QUALIFY = RestrictQualify::disabled>
  class SoAConstValue {
    static_assert(COLUMN_TYPE != SoAColumnType::eigen);

  public:
    using Restr = add_restrict<T, RESTRICT_QUALIFY>;
    using Val = typename Restr::Value;
    using Ptr = typename Restr::Pointer;
    using Ref = typename Restr::Reference;
    using PtrToConst = typename Restr::PointerToConst;
    using RefToConst = typename Restr::ReferenceToConst;
    using Params = SoAParametersImpl<COLUMN_TYPE, T>;
    using ConstParams = SoAConstParametersImpl<COLUMN_TYPE, T>;
    inline __attribute__((always_inline)) SoAConstValue(size_type i, const T* col) : idx_(i), col_(col) {}
    inline __attribute__((always_inline)) SoAConstValue(size_type i, SoAParametersImpl<COLUMN_TYPE, T> params)
        : idx_(i), col_(params.addr_) {}
    inline __attribute__((always_inline)) SoAConstValue(size_type i, SoAConstParametersImpl<COLUMN_TYPE, T> params)
        : idx_(i), col_(params.addr_) {}
    inline __attribute__((always_inline)) RefToConst operator()() const {
      PtrToConst col = col_;
      return col[idx_];
    }
    inline __attribute__((always_inline)) const T* operator&() const { return &col_[idx_]; }
    using valueType = T;
    static constexpr auto valueSize = sizeof(T);

  private:
    size_type idx_;
    const T* col_;
  };
  template <class C, byte_size_type ALIGNMENT, bool RESTRICT_QUALIFY>
  class SoAConstValue<SoAColumnType::eigen, C, ALIGNMENT, RESTRICT_QUALIFY> {
    static_assert(!sizeof(C),
                  "Eigen/Core should be pre-included before the SoA headers to enable support for Eigen columns.");
  };
  template <class C>
  struct EigenConstMapMaker {
    static_assert(!sizeof(C),
                  "Eigen/Core should be pre-included before the SoA headers to enable support for Eigen columns.");
  };
  constexpr inline byte_size_type alignSize(byte_size_type size, byte_size_type alignment) {
    return ((size + alignment - 1) / alignment) * alignment;
  }
}  // namespace cms::soa
namespace cms::soa {
  enum class SoAAccessType : bool { mutableAccess, constAccess };
  template <typename, SoAColumnType, SoAAccessType, byte_size_type, bool>
  struct SoAColumnAccessorsImpl {};
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::column, SoAAccessType::mutableAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAParametersImpl<SoAColumnType::column, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) T* operator()() { return params_.addr_; }
    using NoParamReturnType = T*;
    using ParamReturnType = T&;
    inline __attribute__((always_inline)) T& operator()(size_type index) { return params_.addr_[index]; }

  private:
    SoAParametersImpl<SoAColumnType::column, T> params_;
  };
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::column, SoAAccessType::constAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAConstParametersImpl<SoAColumnType::column, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) const T* operator()() const { return params_.addr_; }
    using NoParamReturnType = const T*;
    using ParamReturnType = const T&;
    inline __attribute__((always_inline)) T const& operator()(size_type index) const { return params_.addr_[index]; }

  private:
    SoAConstParametersImpl<SoAColumnType::column, T> params_;
  };
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::scalar, SoAAccessType::mutableAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAParametersImpl<SoAColumnType::scalar, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) T& operator()() { return *params_.addr_; }
    using NoParamReturnType = T&;
    using ParamReturnType = void;
    inline __attribute__((always_inline)) void operator()(size_type index) const {
      (static_cast<bool>(false && "Indexed access impossible for SoA scalars.")
           ? void(0)
           : __assert_fail(
                 "false && \"Indexed access impossible for SoA scalars.\"",
                 "/data/lebeltra/deep/CMSSW_15_1_X_2025-05-06-1100/src/DataFormats/SoATemplate/interface/SoACommon.h",
                 711,
                 __extension__ __PRETTY_FUNCTION__));
    }

  private:
    SoAParametersImpl<SoAColumnType::scalar, T> params_;
  };
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::scalar, SoAAccessType::constAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAConstParametersImpl<SoAColumnType::scalar, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) T const& operator()() const { return *params_.addr_; }
    using NoParamReturnType = T const&;
    using ParamReturnType = void;
    inline __attribute__((always_inline)) void operator()(size_type index) const {
      (static_cast<bool>(false && "Indexed access impossible for SoA scalars.")
           ? void(0)
           : __assert_fail(
                 "false && \"Indexed access impossible for SoA scalars.\"",
                 "/data/lebeltra/deep/CMSSW_15_1_X_2025-05-06-1100/src/DataFormats/SoATemplate/interface/SoACommon.h",
                 727,
                 __extension__ __PRETTY_FUNCTION__));
    }

  private:
    SoAConstParametersImpl<SoAColumnType::scalar, T> params_;
  };
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::eigen, SoAAccessType::mutableAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAParametersImpl<SoAColumnType::eigen, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) typename T::Scalar* operator()() { return params_.addr_; }
    using NoParamReturnType = typename T::Scalar*;
    using ParamReturnType = typename SoAValue<SoAColumnType::eigen, T, alignment, restrictQualify>::MapType;
    inline __attribute__((always_inline)) ParamReturnType operator()(size_type index) {
      return SoAValue<SoAColumnType::eigen, T, alignment, restrictQualify>(index, params_)();
    }

  private:
    SoAParametersImpl<SoAColumnType::eigen, T> params_;
  };
  template <typename T, byte_size_type alignment, bool restrictQualify>
  struct SoAColumnAccessorsImpl<T, SoAColumnType::eigen, SoAAccessType::constAccess, alignment, restrictQualify> {
    inline __attribute__((always_inline)) SoAColumnAccessorsImpl(
        const SoAConstParametersImpl<SoAColumnType::eigen, T>& params)
        : params_(params) {}
    inline __attribute__((always_inline)) typename T::Scalar const* operator()() const { return params_.addr_; }
    using NoParamReturnType = typename T::Scalar const*;
    using ParamReturnType = typename SoAValue<SoAColumnType::eigen, T, alignment, restrictQualify>::CMapType;
    inline __attribute__((always_inline)) ParamReturnType operator()(size_type index) const {
      return SoAConstValue<SoAColumnType::eigen, T, alignment, restrictQualify>(index, params_)();
    }

  private:
    SoAConstParametersImpl<SoAColumnType::eigen, T> params_;
  };
  template <typename T>
  struct SoAAccessors {
    template <auto columnType>
    struct ColumnType {
      template <auto accessType>
      struct AccessType {
        template <auto alignment>
        struct Alignment {
          template <auto restrictQualify>
          struct RestrictQualifier
              : public SoAColumnAccessorsImpl<T, columnType, accessType, alignment, restrictQualify> {
            using SoAColumnAccessorsImpl<T, columnType, accessType, alignment, restrictQualify>::SoAColumnAccessorsImpl;
          };
        };
      };
    };
  };
  struct AlignmentEnforcement {
    static constexpr bool relaxed = false;
    static constexpr bool enforced = true;
  };
  struct CacheLineSize {
    static constexpr byte_size_type NvidiaGPU = 128;
    static constexpr byte_size_type IntelCPU = 64;
    static constexpr byte_size_type AMDCPU = 64;
    static constexpr byte_size_type ARMCPU = 64;
    static constexpr byte_size_type defaultSize = NvidiaGPU;
  };
}  // namespace cms::soa
template <typename SOA,
          typename SFINAE =
              typename std::enable_if_t<std::is_invocable_v<decltype(&SOA::soaToStreamInternal), SOA&, std::ostream&>>>
std::ostream& operator<<(std::ostream& os, const SOA& soa) {
  soa.soaToStreamInternal(os);
  return os;
}
namespace cms::soa {
  template <class C, SoAColumnType COLUMN_TYPE>
  struct ConstValueTraits : public C {
    using C::C;
  };
  template <class C>
  struct ConstValueTraits<C, SoAColumnType::scalar> {
    inline __attribute__((always_inline)) ConstValueTraits(size_type, const typename C::valueType*) {}
    inline __attribute__((always_inline)) ConstValueTraits(size_type, const typename C::Params&) {}
    inline __attribute__((always_inline)) ConstValueTraits(size_type, const typename C::ConstParams&) {}
  };
}  // namespace cms::soa
template <std::size_t ALIGNMENT = cms::soa::CacheLineSize::defaultSize,
          bool ALIGNMENT_ENFORCEMENT = cms::soa::AlignmentEnforcement::relaxed>
struct SoABlockTemplate {
  using self_type = SoABlockTemplate;
  using AlignmentEnforcement = cms::soa::AlignmentEnforcement;
  using size_type = cms::soa::size_type;
  using byte_size_type = cms::soa::byte_size_type;
  constexpr static byte_size_type defaultAlignment = 128;
  constexpr static byte_size_type alignment = ALIGNMENT;
  constexpr static bool alignmentEnforcement = ALIGNMENT_ENFORCEMENT;
  constexpr static byte_size_type conditionalAlignment =
      alignmentEnforcement == cms::soa::AlignmentEnforcement::enforced ? alignment : 0;
  template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
  using SoAValueWithConf = cms::soa::SoAValue<COLUMN_TYPE, C, conditionalAlignment>;
  template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
  using SoAConstValueWithConf = cms::soa::SoAConstValue<COLUMN_TYPE, C, conditionalAlignment>;
  template <std::size_t VIEW_ALIGNMENT = cms::soa::CacheLineSize::defaultSize,
            bool VIEW_ALIGNMENT_ENFORCEMENT = cms::soa::AlignmentEnforcement::relaxed,
            bool RESTRICT_QUALIFY = cms::soa::RestrictQualify::Default,
            bool RANGE_CHECKING = cms::soa::RangeChecking::Default>
  struct ViewTemplateFreeParams;
  void soaToStreamInternal(std::ostream& _soa_impl_os) const {
    _soa_impl_os << "SoABlockTemplate"
                    "("
                 << elements_ << " elements, byte alignement= " << alignment << ", @" << mem_ << "): " << std::endl;
    _soa_impl_os << "  sizeof("
                    "SoABlockTemplate"
                    "): "
                 << sizeof(SoABlockTemplate) << std::endl;
    byte_size_type _soa_impl_offset = 0;
    _soa_impl_os << " Column "
                    "x"
                    " at offset "
                 << _soa_impl_offset << " has size " << sizeof(float) * elements_ << " and padding "
                 << cms::soa::alignSize(elements_ * sizeof(float), alignment) - (elements_ * sizeof(float))
                 << std::endl;
    _soa_impl_offset += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    _soa_impl_os << " Column "
                    "y"
                    " at offset "
                 << _soa_impl_offset << " has size " << sizeof(float) * elements_ << " and padding "
                 << cms::soa::alignSize(elements_ * sizeof(float), alignment) - (elements_ * sizeof(float))
                 << std::endl;
    _soa_impl_offset += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    _soa_impl_os << " Column "
                    "z"
                    " at offset "
                 << _soa_impl_offset << " has size " << sizeof(float) * elements_ << " and padding "
                 << cms::soa::alignSize(elements_ * sizeof(float), alignment) - (elements_ * sizeof(float))
                 << std::endl;
    _soa_impl_offset += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    _soa_impl_os << " Column "
                    "t"
                    " at offset "
                 << _soa_impl_offset << " has size " << sizeof(float) * elements_ << " and padding "
                 << cms::soa::alignSize(elements_ * sizeof(float), alignment) - (elements_ * sizeof(float))
                 << std::endl;
    _soa_impl_offset += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    _soa_impl_os << " Scalar "
                    "scalar"
                    " at offset "
                 << _soa_impl_offset << " has size " << sizeof(size_t) << " and padding "
                 << ((sizeof(size_t) - 1) / alignment + 1) * alignment - sizeof(size_t) << std::endl;
    _soa_impl_offset += ((sizeof(size_t) - 1) / alignment + 1) * alignment;
    _soa_impl_os << "Final offset = " << _soa_impl_offset << " computeDataSize(...): " << computeDataSize(elements_)
                 << std::endl;
    _soa_impl_os << std::endl;
  }
  static constexpr byte_size_type computeDataSize(size_type elements) {
    byte_size_type _soa_impl_ret = 0;
    _soa_impl_ret += cms::soa::alignSize(elements * sizeof(float), alignment);
    _soa_impl_ret += cms::soa::alignSize(elements * sizeof(float), alignment);
    _soa_impl_ret += cms::soa::alignSize(elements * sizeof(float), alignment);
    _soa_impl_ret += cms::soa::alignSize(elements * sizeof(float), alignment);
    _soa_impl_ret += cms::soa::alignSize(sizeof(size_t), alignment);
    return _soa_impl_ret;
  }
  struct Metadata {
    friend SoABlockTemplate;
    inline __attribute__((always_inline)) size_type size() const { return parent_.elements_; }
    inline __attribute__((always_inline)) byte_size_type byteSize() const { return parent_.byteSize_; }
    inline __attribute__((always_inline)) byte_size_type alignment() const { return SoABlockTemplate::alignment; }
    inline __attribute__((always_inline)) std::byte* data() { return parent_.mem_; }
    inline __attribute__((always_inline)) const std::byte* data() const { return parent_.mem_; }
    inline __attribute__((always_inline)) std::byte* nextByte() const { return parent_.mem_ + parent_.byteSize_; }
    inline __attribute__((always_inline)) SoABlockTemplate cloneToNewAddress(std::byte* _soa_impl_addr) const {
      return SoABlockTemplate(_soa_impl_addr, parent_.elements_);
    }
    using ParametersTypeOf_x = cms::soa::SoAParameters_ColumnType<cms::soa::SoAColumnType::column>::DataType<float>;
    inline __attribute__((always_inline)) ParametersTypeOf_x parametersOf_x() const {
      return ParametersTypeOf_x(parent_.x_, parent_.metadata().size());
    }
    inline __attribute__((always_inline)) float const* addressOf_x() const {
      return parent_.metadata().parametersOf_x().addr_;
    }
    inline __attribute__((always_inline)) float* addressOf_x() { return parent_.metadata().parametersOf_x().addr_; }
    inline __attribute__((always_inline)) byte_size_type xPitch() const {
      return cms::soa::alignSize(parent_.elements_ * sizeof(float), ParentClass::alignment);
    }
    using TypeOf_x = float;
    constexpr static cms::soa::SoAColumnType ColumnTypeOf_x = cms::soa::SoAColumnType::column;
    using ParametersTypeOf_y = cms::soa::SoAParameters_ColumnType<cms::soa::SoAColumnType::column>::DataType<float>;
    inline __attribute__((always_inline)) ParametersTypeOf_y parametersOf_y() const {
      return ParametersTypeOf_y(parent_.y_, parent_.metadata().size());
    }
    inline __attribute__((always_inline)) float const* addressOf_y() const {
      return parent_.metadata().parametersOf_y().addr_;
    }
    inline __attribute__((always_inline)) float* addressOf_y() { return parent_.metadata().parametersOf_y().addr_; }
    inline __attribute__((always_inline)) byte_size_type yPitch() const {
      return cms::soa::alignSize(parent_.elements_ * sizeof(float), ParentClass::alignment);
    }
    using TypeOf_y = float;
    constexpr static cms::soa::SoAColumnType ColumnTypeOf_y = cms::soa::SoAColumnType::column;
    using ParametersTypeOf_z = cms::soa::SoAParameters_ColumnType<cms::soa::SoAColumnType::column>::DataType<float>;
    inline __attribute__((always_inline)) ParametersTypeOf_z parametersOf_z() const {
      return ParametersTypeOf_z(parent_.z_, parent_.metadata().size());
    }
    inline __attribute__((always_inline)) float const* addressOf_z() const {
      return parent_.metadata().parametersOf_z().addr_;
    }
    inline __attribute__((always_inline)) float* addressOf_z() { return parent_.metadata().parametersOf_z().addr_; }
    inline __attribute__((always_inline)) byte_size_type zPitch() const {
      return cms::soa::alignSize(parent_.elements_ * sizeof(float), ParentClass::alignment);
    }
    using TypeOf_z = float;
    constexpr static cms::soa::SoAColumnType ColumnTypeOf_z = cms::soa::SoAColumnType::column;
    using ParametersTypeOf_t = cms::soa::SoAParameters_ColumnType<cms::soa::SoAColumnType::column>::DataType<float>;
    inline __attribute__((always_inline)) ParametersTypeOf_t parametersOf_t() const {
      return ParametersTypeOf_t(parent_.t_, parent_.metadata().size());
    }
    inline __attribute__((always_inline)) float const* addressOf_t() const {
      return parent_.metadata().parametersOf_t().addr_;
    }
    inline __attribute__((always_inline)) float* addressOf_t() { return parent_.metadata().parametersOf_t().addr_; }
    inline __attribute__((always_inline)) byte_size_type tPitch() const {
      return cms::soa::alignSize(parent_.elements_ * sizeof(float), ParentClass::alignment);
    }
    using TypeOf_t = float;
    constexpr static cms::soa::SoAColumnType ColumnTypeOf_t = cms::soa::SoAColumnType::column;
    byte_size_type scalarPitch() const { return cms::soa::alignSize(sizeof(size_t), ParentClass::alignment); }
    using TypeOf_scalar = size_t;
    constexpr static cms::soa::SoAColumnType ColumnTypeOf_scalar = cms::soa::SoAColumnType::scalar;
    inline __attribute__((always_inline)) size_t const* addressOf_scalar() const {
      return parent_.metadata().parametersOf_scalar().addr_;
    }
    using ParametersTypeOf_scalar =
        cms::soa::SoAParameters_ColumnType<cms::soa::SoAColumnType::scalar>::DataType<size_t>;
    inline __attribute__((always_inline)) ParametersTypeOf_scalar parametersOf_scalar() const {
      return ParametersTypeOf_scalar(parent_.scalar_, parent_.metadata().size());
    }
    inline __attribute__((always_inline)) size_t* addressOf_scalar() {
      return parent_.metadata().parametersOf_scalar().addr_;
    }
    struct value_element {
      inline __attribute__((always_inline)) value_element(float x, float y, float z, float t)
          : () x{x}, y{y}, z{z}, t{t} {}
      float x;
      float y;
      float z;
      float t;
    };
    Metadata& operator=(const Metadata&) = delete;
    Metadata(const Metadata&) = delete;

  private:
    inline __attribute__((always_inline)) Metadata(const SoABlockTemplate& _soa_impl_parent)
        : parent_(_soa_impl_parent) {}
    const SoABlockTemplate& parent_;
    using ParentClass = SoABlockTemplate;
  };
  friend Metadata;
  inline __attribute__((always_inline)) const Metadata metadata() const { return Metadata(*this); }
  inline __attribute__((always_inline)) Metadata metadata() { return Metadata(*this); }
  template <std::size_t VIEW_ALIGNMENT, bool VIEW_ALIGNMENT_ENFORCEMENT, bool RESTRICT_QUALIFY, bool RANGE_CHECKING>
  struct ConstViewTemplateFreeParams {
    using self_type = ConstViewTemplateFreeParams;
    using SoABlockTemplate_parametrized = SoABlockTemplate<VIEW_ALIGNMENT, VIEW_ALIGNMENT_ENFORCEMENT>;
    using size_type = cms::soa::size_type;
    using byte_size_type = cms::soa::byte_size_type;
    using AlignmentEnforcement = cms::soa::AlignmentEnforcement;
    template <std::size_t, bool, bool, bool>
    friend struct ViewTemplateFreeParams;
    template <std::size_t, bool, bool, bool>
    friend struct ConstViewTemplateFreeParams;
    constexpr static byte_size_type defaultAlignment = cms::soa::CacheLineSize::defaultSize;
    constexpr static byte_size_type alignment = VIEW_ALIGNMENT;
    constexpr static bool alignmentEnforcement = VIEW_ALIGNMENT_ENFORCEMENT;
    constexpr static byte_size_type conditionalAlignment =
        alignmentEnforcement == AlignmentEnforcement::enforced ? alignment : 0;
    constexpr static bool restrictQualify = RESTRICT_QUALIFY;
    constexpr static bool rangeChecking = RANGE_CHECKING;
    template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
    using SoAValueWithConf = cms::soa::SoAValue<COLUMN_TYPE, C, conditionalAlignment, restrictQualify>;
    template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
    using SoAConstValueWithConf = cms::soa::SoAConstValue<COLUMN_TYPE, C, conditionalAlignment, restrictQualify>;
    struct Metadata {
      friend ConstViewTemplateFreeParams;
      inline __attribute__((always_inline)) size_type size() const { return parent_.elements_; }
      using TypeOf_instance_SoABlockTemplate = SoABlockTemplate_parametrized;
      using TypeOf_x = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_x;
      using ParametersTypeOf_x = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_x;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_x =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_x;
      using ConstAccessorOf_x = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_x = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_x() const { return (parent_.xParameters_); };
      using TypeOf_y = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_y;
      using ParametersTypeOf_y = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_y;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_y =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_y;
      using ConstAccessorOf_y = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_y = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_y() const { return (parent_.yParameters_); };
      using TypeOf_z = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_z;
      using ParametersTypeOf_z = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_z;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_z =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_z;
      using ConstAccessorOf_z = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_z = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_z() const { return (parent_.zParameters_); };
      using TypeOf_t = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_t;
      using ParametersTypeOf_t = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_t;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_t =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_t;
      using ConstAccessorOf_t = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_t = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_t() const { return (parent_.tParameters_); };
      inline __attribute__((always_inline)) auto const* addressOf_x() const { return parametersOf_x().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_y() const { return parametersOf_y().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_z() const { return parametersOf_z().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_t() const { return parametersOf_t().addr_; };
      Metadata& operator=(const Metadata&) = delete;
      Metadata(const Metadata&) = delete;

    private:
      inline __attribute__((always_inline)) Metadata(const ConstViewTemplateFreeParams& _soa_impl_parent)
          : parent_(_soa_impl_parent) {}
      const ConstViewTemplateFreeParams& parent_;
    };
    friend Metadata;
    struct Metarecords {
      friend ConstViewTemplateFreeParams;
      Metarecords(const ConstViewTemplateFreeParams& _soa_impl_parent)
          : parent_(_soa_impl_parent),
            x_{parent_.metadata().parametersOf_x()},
            y_{parent_.metadata().parametersOf_y()},
            z_{parent_.metadata().parametersOf_z()},
            t_{parent_.metadata().parametersOf_t()} {}
      const typename Metadata::ParametersTypeOf_x::ConstType& x() const { return x_; }
      const typename Metadata::ParametersTypeOf_y::ConstType& y() const { return y_; }
      const typename Metadata::ParametersTypeOf_z::ConstType& z() const { return z_; }
      const typename Metadata::ParametersTypeOf_t::ConstType& t() const { return t_; }

    private:
      const ConstViewTemplateFreeParams& parent_;
      typename Metadata::ParametersTypeOf_x::ConstType x_;
      typename Metadata::ParametersTypeOf_y::ConstType y_;
      typename Metadata::ParametersTypeOf_z::ConstType z_;
      typename Metadata::ParametersTypeOf_t::ConstType t_;
    };
    inline __attribute__((always_inline)) const Metadata metadata() const { return Metadata(*this); }
    inline __attribute__((always_inline)) const Metarecords records() const { return Metarecords(*this); }
    ConstViewTemplateFreeParams() = default;
    ConstViewTemplateFreeParams(const SoABlockTemplate_parametrized& instance_SoABlockTemplate)
        : elements_([&]() -> size_type {
            bool set = false;
            size_type ret = 0;
            if (set) {
              if (ret != instance_SoABlockTemplate.metadata().size())
                throw std::runtime_error("In constructor by layout: different sizes from layouts.");
            } else {
              ret = instance_SoABlockTemplate.metadata().size();
              set = true;
            }
            return ret;
          }()),
          xParameters_([&]() -> auto {
            auto params = instance_SoABlockTemplate.metadata().parametersOf_x();
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (reinterpret_cast<intptr_t>(params.addr_) % alignment)
                throw std::runtime_error(
                    "In constructor by layout: misaligned column: "
                    "x");
            return params;
          }()),
          yParameters_([&]() -> auto {
            auto params = instance_SoABlockTemplate.metadata().parametersOf_y();
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (reinterpret_cast<intptr_t>(params.addr_) % alignment)
                throw std::runtime_error(
                    "In constructor by layout: misaligned column: "
                    "y");
            return params;
          }()),
          zParameters_([&]() -> auto {
            auto params = instance_SoABlockTemplate.metadata().parametersOf_z();
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (reinterpret_cast<intptr_t>(params.addr_) % alignment)
                throw std::runtime_error(
                    "In constructor by layout: misaligned column: "
                    "z");
            return params;
          }()),
          tParameters_([&]() -> auto {
            auto params = instance_SoABlockTemplate.metadata().parametersOf_t();
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (reinterpret_cast<intptr_t>(params.addr_) % alignment)
                throw std::runtime_error(
                    "In constructor by layout: misaligned column: "
                    "t");
            return params;
          }()) {}
    ConstViewTemplateFreeParams(size_type _soa_impl_elements,
                                const typename Metadata::ParametersTypeOf_x::TupleOrPointerType x,
                                const typename Metadata::ParametersTypeOf_y::TupleOrPointerType y,
                                const typename Metadata::ParametersTypeOf_z::TupleOrPointerType z,
                                const typename Metadata::ParametersTypeOf_t::TupleOrPointerType t)
        : elements_(_soa_impl_elements),
          xParameters_([&]() -> auto {
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (Metadata::ParametersTypeOf_x::checkAlignment(x, alignment))
                throw std::runtime_error(
                    "In constructor by column: misaligned column: "
                    "x");
            return x;
          }()),
          yParameters_([&]() -> auto {
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (Metadata::ParametersTypeOf_y::checkAlignment(y, alignment))
                throw std::runtime_error(
                    "In constructor by column: misaligned column: "
                    "y");
            return y;
          }()),
          zParameters_([&]() -> auto {
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (Metadata::ParametersTypeOf_z::checkAlignment(z, alignment))
                throw std::runtime_error(
                    "In constructor by column: misaligned column: "
                    "z");
            return z;
          }()),
          tParameters_([&]() -> auto {
            if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
              if (Metadata::ParametersTypeOf_t::checkAlignment(t, alignment))
                throw std::runtime_error(
                    "In constructor by column: misaligned column: "
                    "t");
            return t;
          }()) {}
    ConstViewTemplateFreeParams(typename Metadata::ParametersTypeOf_x::ConstType x,
                                typename Metadata::ParametersTypeOf_y::ConstType y,
                                typename Metadata::ParametersTypeOf_z::ConstType z,
                                typename Metadata::ParametersTypeOf_t::ConstType t) {
      bool readyToSet = false;
      if (not readyToSet) {
        elements_ = x.size_;
        readyToSet = true;
      }
      auto x_tmp = [&]() -> auto {
        if (elements_ != x.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "x");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_x::checkAlignment(x, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "x");
        return x;
      }();
      xParameters_ = x_tmp;
      if (not readyToSet) {
        elements_ = y.size_;
        readyToSet = true;
      }
      auto y_tmp = [&]() -> auto {
        if (elements_ != y.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "y");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_y::checkAlignment(y, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "y");
        return y;
      }();
      yParameters_ = y_tmp;
      if (not readyToSet) {
        elements_ = z.size_;
        readyToSet = true;
      }
      auto z_tmp = [&]() -> auto {
        if (elements_ != z.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "z");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_z::checkAlignment(z, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "z");
        return z;
      }();
      zParameters_ = z_tmp;
      if (not readyToSet) {
        elements_ = t.size_;
        readyToSet = true;
      }
      auto t_tmp = [&]() -> auto {
        if (elements_ != t.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "t");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_t::checkAlignment(t, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "t");
        return t;
      }();
      tParameters_ = t_tmp;
    }
    ConstViewTemplateFreeParams(ConstViewTemplateFreeParams const&) = default;
    ConstViewTemplateFreeParams& operator=(ConstViewTemplateFreeParams const&) = default;
    template <std::size_t OTHER_VIEW_ALIGNMENT,
              bool OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
              bool OTHER_RESTRICT_QUALIFY,
              bool OTHER_RANGE_CHECKING>
    ConstViewTemplateFreeParams(ConstViewTemplateFreeParams<OTHER_VIEW_ALIGNMENT,
                                                            OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
                                                            OTHER_RESTRICT_QUALIFY,
                                                            OTHER_RANGE_CHECKING> const& other)
        : ConstViewTemplateFreeParams{other.elements_,
                                      const_cast_SoAParametersImpl(other.xParameters_).tupleOrPointer(),
                                      const_cast_SoAParametersImpl(other.yParameters_).tupleOrPointer(),
                                      const_cast_SoAParametersImpl(other.zParameters_).tupleOrPointer(),
                                      const_cast_SoAParametersImpl(other.tParameters_).tupleOrPointer()} {}
    template <std::size_t OTHER_VIEW_ALIGNMENT,
              bool OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
              bool OTHER_RESTRICT_QUALIFY,
              bool OTHER_RANGE_CHECKING>
    ConstViewTemplateFreeParams& operator=(ConstViewTemplateFreeParams<OTHER_VIEW_ALIGNMENT,
                                                                       OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
                                                                       OTHER_RESTRICT_QUALIFY,
                                                                       OTHER_RANGE_CHECKING> const& other) {
      *this = other;
    }
    ConstViewTemplateFreeParams(ConstViewTemplateFreeParams&&) = default;
    ConstViewTemplateFreeParams& operator=(ConstViewTemplateFreeParams&&) = default;
    ~ConstViewTemplateFreeParams() = default;
    struct const_element {
      inline __attribute__((always_inline)) const_element(size_type _soa_impl_index,
                                                          const typename Metadata::ParametersTypeOf_x::ConstType x,
                                                          const typename Metadata::ParametersTypeOf_y::ConstType y,
                                                          const typename Metadata::ParametersTypeOf_z::ConstType z,
                                                          const typename Metadata::ParametersTypeOf_t::ConstType t)
          : x_(_soa_impl_index, x), y_(_soa_impl_index, y), z_(_soa_impl_index, z), t_(_soa_impl_index, t) {}
      inline __attribute__((always_inline))
      const typename SoAConstValueWithConf<Metadata::ColumnTypeOf_x, const typename Metadata::TypeOf_x>::RefToConst
      x() const {
        return x_();
      }
      inline __attribute__((always_inline))
      const typename SoAConstValueWithConf<Metadata::ColumnTypeOf_y, const typename Metadata::TypeOf_y>::RefToConst
      y() const {
        return y_();
      }
      inline __attribute__((always_inline))
      const typename SoAConstValueWithConf<Metadata::ColumnTypeOf_z, const typename Metadata::TypeOf_z>::RefToConst
      z() const {
        return z_();
      }
      inline __attribute__((always_inline))
      const typename SoAConstValueWithConf<Metadata::ColumnTypeOf_t, const typename Metadata::TypeOf_t>::RefToConst
      t() const {
        return t_();
      }

    private:
      const cms::soa::ConstValueTraits<SoAConstValueWithConf<Metadata::ColumnTypeOf_x, typename Metadata::TypeOf_x>,
                                       Metadata::ColumnTypeOf_x>
          x_;
      const cms::soa::ConstValueTraits<SoAConstValueWithConf<Metadata::ColumnTypeOf_y, typename Metadata::TypeOf_y>,
                                       Metadata::ColumnTypeOf_y>
          y_;
      const cms::soa::ConstValueTraits<SoAConstValueWithConf<Metadata::ColumnTypeOf_z, typename Metadata::TypeOf_z>,
                                       Metadata::ColumnTypeOf_z>
          z_;
      const cms::soa::ConstValueTraits<SoAConstValueWithConf<Metadata::ColumnTypeOf_t, typename Metadata::TypeOf_t>,
                                       Metadata::ColumnTypeOf_t>
          t_;
    };
    inline __attribute__((always_inline)) const_element operator[](size_type _soa_impl_index) const {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in "
              "ConstViewTemplateFreeParams"
              "::operator[]");
        }
      }
      return const_element{_soa_impl_index, xParameters_, yParameters_, zParameters_, tParameters_};
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<Metadata::ColumnTypeOf_x>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        x() const {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          Metadata::ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(xParameters_)();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<Metadata::ColumnTypeOf_x>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        x(size_type _soa_impl_index) const {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in const "
              "x"
              "(size_type index)");
        }
      }
      return
          typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<Metadata::ColumnTypeOf_x>::
              template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
                  conditionalAlignment>::template RestrictQualifier<restrictQualify>(xParameters_)(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<Metadata::ColumnTypeOf_y>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        y() const {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          Metadata::ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(yParameters_)();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<Metadata::ColumnTypeOf_y>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        y(size_type _soa_impl_index) const {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in const "
              "y"
              "(size_type index)");
        }
      }
      return
          typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<Metadata::ColumnTypeOf_y>::
              template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
                  conditionalAlignment>::template RestrictQualifier<restrictQualify>(yParameters_)(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<Metadata::ColumnTypeOf_z>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        z() const {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          Metadata::ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(zParameters_)();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<Metadata::ColumnTypeOf_z>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        z(size_type _soa_impl_index) const {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in const "
              "z"
              "(size_type index)");
        }
      }
      return
          typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<Metadata::ColumnTypeOf_z>::
              template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
                  conditionalAlignment>::template RestrictQualifier<restrictQualify>(zParameters_)(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<Metadata::ColumnTypeOf_t>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        t() const {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          Metadata::ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(tParameters_)();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<Metadata::ColumnTypeOf_t>::
        template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        t(size_type _soa_impl_index) const {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in const "
              "t"
              "(size_type index)");
        }
      }
      return
          typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<Metadata::ColumnTypeOf_t>::
              template AccessType<cms::soa::SoAAccessType::constAccess>::template Alignment<
                  conditionalAlignment>::template RestrictQualifier<restrictQualify>(tParameters_)(_soa_impl_index);
    }
    template <typename T>
    friend void dump();

  private:
    size_type elements_ = 0;
    typename Metadata::ParametersTypeOf_x::ConstType xParameters_;
    typename Metadata::ParametersTypeOf_y::ConstType yParameters_;
    typename Metadata::ParametersTypeOf_z::ConstType zParameters_;
    typename Metadata::ParametersTypeOf_t::ConstType tParameters_;
  };
  template <bool RESTRICT_QUALIFY, bool RANGE_CHECKING>
  using ConstViewTemplate =
      ConstViewTemplateFreeParams<ALIGNMENT, ALIGNMENT_ENFORCEMENT, RESTRICT_QUALIFY, RANGE_CHECKING>;
  using ConstView = ConstViewTemplate<cms::soa::RestrictQualify::Default, cms::soa::RangeChecking::Default>;
  template <std::size_t VIEW_ALIGNMENT, bool VIEW_ALIGNMENT_ENFORCEMENT, bool RESTRICT_QUALIFY, bool RANGE_CHECKING>
  struct ViewTemplateFreeParams
      : public ConstViewTemplateFreeParams<VIEW_ALIGNMENT, VIEW_ALIGNMENT_ENFORCEMENT, RESTRICT_QUALIFY, RANGE_CHECKING> {
    using self_type = ViewTemplateFreeParams;
    using base_type =
        ConstViewTemplateFreeParams<VIEW_ALIGNMENT, VIEW_ALIGNMENT_ENFORCEMENT, RESTRICT_QUALIFY, RANGE_CHECKING>;
    using SoABlockTemplate_parametrized = SoABlockTemplate<VIEW_ALIGNMENT, VIEW_ALIGNMENT_ENFORCEMENT>;
    using size_type = cms::soa::size_type;
    using byte_size_type = cms::soa::byte_size_type;
    using AlignmentEnforcement = cms::soa::AlignmentEnforcement;
    constexpr static byte_size_type defaultAlignment = cms::soa::CacheLineSize::defaultSize;
    constexpr static byte_size_type alignment = VIEW_ALIGNMENT;
    constexpr static bool alignmentEnforcement = VIEW_ALIGNMENT_ENFORCEMENT;
    constexpr static byte_size_type conditionalAlignment =
        alignmentEnforcement == AlignmentEnforcement::enforced ? alignment : 0;
    constexpr static bool restrictQualify = RESTRICT_QUALIFY;
    constexpr static bool rangeChecking = RANGE_CHECKING;
    template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
    using SoAValueWithConf = cms::soa::SoAValue<COLUMN_TYPE, C, conditionalAlignment, restrictQualify>;
    template <cms::soa::SoAColumnType COLUMN_TYPE, class C>
    using SoAConstValueWithConf = cms::soa::SoAConstValue<COLUMN_TYPE, C, conditionalAlignment, restrictQualify>;
    template <std::size_t, bool, bool, bool>
    friend struct ViewTemplateFreeParams;
    struct Metadata {
      friend ViewTemplateFreeParams;
      inline __attribute__((always_inline)) size_type size() const { return parent_.elements_; }
      using TypeOf_instance_SoABlockTemplate = SoABlockTemplate_parametrized;
      using TypeOf_x = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_x;
      using ParametersTypeOf_x = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_x;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_x =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_x;
      using ConstAccessorOf_x = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_x = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_x() const {
        return const_cast_SoAParametersImpl(parent_.xParameters_);
      };
      using TypeOf_y = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_y;
      using ParametersTypeOf_y = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_y;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_y =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_y;
      using ConstAccessorOf_y = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_y = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_y() const {
        return const_cast_SoAParametersImpl(parent_.yParameters_);
      };
      using TypeOf_z = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_z;
      using ParametersTypeOf_z = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_z;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_z =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_z;
      using ConstAccessorOf_z = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_z = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_z() const {
        return const_cast_SoAParametersImpl(parent_.zParameters_);
      };
      using TypeOf_t = typename TypeOf_instance_SoABlockTemplate::Metadata::TypeOf_t;
      using ParametersTypeOf_t = typename TypeOf_instance_SoABlockTemplate::Metadata::ParametersTypeOf_t;
      constexpr static cms::soa::SoAColumnType ColumnTypeOf_t =
          TypeOf_instance_SoABlockTemplate::Metadata::ColumnTypeOf_t;
      using ConstAccessorOf_t = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::constAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      using MutableAccessorOf_t = typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>;
      inline __attribute__((always_inline)) const auto parametersOf_t() const {
        return const_cast_SoAParametersImpl(parent_.tParameters_);
      };
      inline __attribute__((always_inline)) auto* addressOf_x() { return parametersOf_x().addr_; };
      inline __attribute__((always_inline)) auto* addressOf_y() { return parametersOf_y().addr_; };
      inline __attribute__((always_inline)) auto* addressOf_z() { return parametersOf_z().addr_; };
      inline __attribute__((always_inline)) auto* addressOf_t() { return parametersOf_t().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_x() const { return parametersOf_x().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_y() const { return parametersOf_y().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_z() const { return parametersOf_z().addr_; };
      inline __attribute__((always_inline)) auto const* addressOf_t() const { return parametersOf_t().addr_; };
      Metadata& operator=(const Metadata&) = delete;
      Metadata(const Metadata&) = delete;

    private:
      inline __attribute__((always_inline)) Metadata(const ViewTemplateFreeParams& _soa_impl_parent)
          : parent_(_soa_impl_parent) {}
      const ViewTemplateFreeParams& parent_;
    };
    friend Metadata;
    struct Metarecords {
      friend ViewTemplateFreeParams;
      Metarecords(const ViewTemplateFreeParams& _soa_impl_parent)
          : parent_(_soa_impl_parent),
            x_{parent_.metadata().parametersOf_x()},
            y_{parent_.metadata().parametersOf_y()},
            z_{parent_.metadata().parametersOf_z()},
            t_{parent_.metadata().parametersOf_t()} {}
      const typename Metadata::ParametersTypeOf_x& x() const { return x_; }
      const typename Metadata::ParametersTypeOf_y& y() const { return y_; }
      const typename Metadata::ParametersTypeOf_z& z() const { return z_; }
      const typename Metadata::ParametersTypeOf_t& t() const { return t_; }

    private:
      const ViewTemplateFreeParams& parent_;
      typename Metadata::ParametersTypeOf_x x_;
      typename Metadata::ParametersTypeOf_y y_;
      typename Metadata::ParametersTypeOf_z z_;
      typename Metadata::ParametersTypeOf_t t_;
    };
    inline __attribute__((always_inline)) const Metadata metadata() const { return Metadata(*this); }
    inline __attribute__((always_inline)) Metadata metadata() { return Metadata(*this); }
    inline __attribute__((always_inline)) const Metarecords records() const { return Metarecords(*this); }
    inline __attribute__((always_inline)) Metarecords records() { return Metarecords(*this); }
    ViewTemplateFreeParams() = default;
    ViewTemplateFreeParams(SoABlockTemplate_parametrized& instance_SoABlockTemplate)
        : base_type{instance_SoABlockTemplate} {}
    ViewTemplateFreeParams(size_type _soa_impl_elements,
                           typename Metadata::ParametersTypeOf_x::TupleOrPointerType x,
                           typename Metadata::ParametersTypeOf_y::TupleOrPointerType y,
                           typename Metadata::ParametersTypeOf_z::TupleOrPointerType z,
                           typename Metadata::ParametersTypeOf_t::TupleOrPointerType t)
        : base_type{_soa_impl_elements, x, y, z, t} {}
    ViewTemplateFreeParams(typename Metadata::ParametersTypeOf_x x,
                           typename Metadata::ParametersTypeOf_y y,
                           typename Metadata::ParametersTypeOf_z z,
                           typename Metadata::ParametersTypeOf_t t) {
      bool readyToSet = false;
      if (not readyToSet) {
        base_type::elements_ = x.size_;
        readyToSet = true;
      }
      auto x_tmp = [&]() -> auto {
        if (base_type::elements_ != x.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "x");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_x::checkAlignment(x, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "x");
        return x;
      }();
      base_type::xParameters_ = x_tmp;
      if (not readyToSet) {
        base_type::elements_ = y.size_;
        readyToSet = true;
      }
      auto y_tmp = [&]() -> auto {
        if (base_type::elements_ != y.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "y");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_y::checkAlignment(y, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "y");
        return y;
      }();
      base_type::yParameters_ = y_tmp;
      if (not readyToSet) {
        base_type::elements_ = z.size_;
        readyToSet = true;
      }
      auto z_tmp = [&]() -> auto {
        if (base_type::elements_ != z.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "z");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_z::checkAlignment(z, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "z");
        return z;
      }();
      base_type::zParameters_ = z_tmp;
      if (not readyToSet) {
        base_type::elements_ = t.size_;
        readyToSet = true;
      }
      auto t_tmp = [&]() -> auto {
        if (base_type::elements_ != t.size_)
          throw std::runtime_error(
              "In constructor by column pointers: number of elements not equal for every column: "
              "t");
        if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
          if (Metadata::ParametersTypeOf_t::checkAlignment(t, alignment))
            throw std::runtime_error(
                "In constructor by column: misaligned column: "
                "t");
        return t;
      }();
      base_type::tParameters_ = t_tmp;
    }
    ViewTemplateFreeParams(ViewTemplateFreeParams const&) = default;
    ViewTemplateFreeParams& operator=(ViewTemplateFreeParams const&) = default;
    template <std::size_t OTHER_VIEW_ALIGNMENT,
              bool OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
              bool OTHER_RESTRICT_QUALIFY,
              bool OTHER_RANGE_CHECKING>
    ViewTemplateFreeParams(ViewTemplateFreeParams<OTHER_VIEW_ALIGNMENT,
                                                  OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
                                                  OTHER_RESTRICT_QUALIFY,
                                                  OTHER_RANGE_CHECKING> const& other)
        : base_type{other.elements_,
                    const_cast_SoAParametersImpl(other.xParameters_).tupleOrPointer(),
                    const_cast_SoAParametersImpl(other.yParameters_).tupleOrPointer(),
                    const_cast_SoAParametersImpl(other.zParameters_).tupleOrPointer(),
                    const_cast_SoAParametersImpl(other.tParameters_).tupleOrPointer()} {}
    template <std::size_t OTHER_VIEW_ALIGNMENT,
              bool OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
              bool OTHER_RESTRICT_QUALIFY,
              bool OTHER_RANGE_CHECKING>
    ViewTemplateFreeParams& operator=(ViewTemplateFreeParams<OTHER_VIEW_ALIGNMENT,
                                                             OTHER_VIEW_ALIGNMENT_ENFORCEMENT,
                                                             OTHER_RESTRICT_QUALIFY,
                                                             OTHER_RANGE_CHECKING> const& other) {
      static_cast<base_type>(*this) = static_cast<base_type>(other);
    }
    ViewTemplateFreeParams(ViewTemplateFreeParams&&) = default;
    ViewTemplateFreeParams& operator=(ViewTemplateFreeParams&&) = default;
    ~ViewTemplateFreeParams() = default;
    using const_element = typename base_type::const_element;
    using base_type::operator[];
    struct element {
      inline __attribute__((always_inline)) element(size_type _soa_impl_index,
                                                    typename Metadata::ParametersTypeOf_x x,
                                                    typename Metadata::ParametersTypeOf_y y,
                                                    typename Metadata::ParametersTypeOf_z z,
                                                    typename Metadata::ParametersTypeOf_t t)
          : x(_soa_impl_index, x), y(_soa_impl_index, y), z(_soa_impl_index, z), t(_soa_impl_index, t) {}
      inline __attribute__((always_inline)) element& operator=(const element& _soa_impl_other) {
        if constexpr (Metadata::ColumnTypeOf_x != cms::soa::SoAColumnType::scalar)
          x() = _soa_impl_other.x();
        if constexpr (Metadata::ColumnTypeOf_y != cms::soa::SoAColumnType::scalar)
          y() = _soa_impl_other.y();
        if constexpr (Metadata::ColumnTypeOf_z != cms::soa::SoAColumnType::scalar)
          z() = _soa_impl_other.z();
        if constexpr (Metadata::ColumnTypeOf_t != cms::soa::SoAColumnType::scalar)
          t() = _soa_impl_other.t();
        return *this;
      }
      inline __attribute__((always_inline)) element& operator=(const const_element& _soa_impl_other) {
        if constexpr (Metadata::ColumnTypeOf_x != cms::soa::SoAColumnType::scalar)
          x() = _soa_impl_other.x();
        if constexpr (Metadata::ColumnTypeOf_y != cms::soa::SoAColumnType::scalar)
          y() = _soa_impl_other.y();
        if constexpr (Metadata::ColumnTypeOf_z != cms::soa::SoAColumnType::scalar)
          z() = _soa_impl_other.z();
        if constexpr (Metadata::ColumnTypeOf_t != cms::soa::SoAColumnType::scalar)
          t() = _soa_impl_other.t();
        return *this;
      }
      inline __attribute__((always_inline)) constexpr element& operator=(
          const typename SoABlockTemplate_parametrized::Metadata::value_element _soa_impl_value) {
        x() = _soa_impl_value.x;
        y() = _soa_impl_value.y;
        z() = _soa_impl_value.z;
        t() = _soa_impl_value.t;
        return *this;
      }
      SoAValueWithConf<Metadata::ColumnTypeOf_x, typename Metadata::TypeOf_x> x;
      SoAValueWithConf<Metadata::ColumnTypeOf_y, typename Metadata::TypeOf_y> y;
      SoAValueWithConf<Metadata::ColumnTypeOf_z, typename Metadata::TypeOf_z> z;
      SoAValueWithConf<Metadata::ColumnTypeOf_t, typename Metadata::TypeOf_t> t;
    };
    inline __attribute__((always_inline)) element operator[](size_type _soa_impl_index) {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= base_type::elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in "
              "ViewTemplateFreeParams"
              "::operator[]");
        }
      }
      return element{_soa_impl_index,
                     const_cast_SoAParametersImpl(base_type::xParameters_),
                     const_cast_SoAParametersImpl(base_type::yParameters_),
                     const_cast_SoAParametersImpl(base_type::zParameters_),
                     const_cast_SoAParametersImpl(base_type::tParameters_)};
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<Metadata::ColumnTypeOf_x>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        x() {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          Metadata::ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::xParameters_))();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<Metadata::ColumnTypeOf_x>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        x(size_type _soa_impl_index) {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= base_type::elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in mutable "
              "x"
              "(size_type index)");
        }
      }
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_x>::template ColumnType<
          Metadata::ColumnTypeOf_x>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::xParameters_))(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<Metadata::ColumnTypeOf_y>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        y() {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          Metadata::ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::yParameters_))();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<Metadata::ColumnTypeOf_y>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        y(size_type _soa_impl_index) {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= base_type::elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in mutable "
              "y"
              "(size_type index)");
        }
      }
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_y>::template ColumnType<
          Metadata::ColumnTypeOf_y>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::yParameters_))(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<Metadata::ColumnTypeOf_z>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        z() {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          Metadata::ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::zParameters_))();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<Metadata::ColumnTypeOf_z>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        z(size_type _soa_impl_index) {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= base_type::elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in mutable "
              "z"
              "(size_type index)");
        }
      }
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_z>::template ColumnType<
          Metadata::ColumnTypeOf_z>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::zParameters_))(_soa_impl_index);
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<Metadata::ColumnTypeOf_t>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::NoParamReturnType
        t() {
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          Metadata::ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::tParameters_))();
    }
    inline __attribute__((always_inline))
    typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<Metadata::ColumnTypeOf_t>::
        template AccessType<cms::soa::SoAAccessType::mutableAccess>::template Alignment<
            conditionalAlignment>::template RestrictQualifier<restrictQualify>::ParamReturnType
        t(size_type _soa_impl_index) {
      if constexpr (rangeChecking == cms::soa::RangeChecking::enabled) {
        if (_soa_impl_index >= base_type::elements_ or _soa_impl_index < 0) {
          throw std::out_of_range(
              "Out of range index in mutable "
              "t"
              "(size_type index)");
        }
      }
      return typename cms::soa::SoAAccessors<typename Metadata::TypeOf_t>::template ColumnType<
          Metadata::ColumnTypeOf_t>::template AccessType<cms::soa::SoAAccessType::mutableAccess>::
          template Alignment<conditionalAlignment>::template RestrictQualifier<restrictQualify>(
              const_cast_SoAParametersImpl(base_type::tParameters_))(_soa_impl_index);
    }
    template <typename T>
    friend void dump();
  };
  template <bool RESTRICT_QUALIFY, bool RANGE_CHECKING>
  using ViewTemplate = ViewTemplateFreeParams<ALIGNMENT, ALIGNMENT_ENFORCEMENT, RESTRICT_QUALIFY, RANGE_CHECKING>;
  using View = ViewTemplate<cms::soa::RestrictQualify::Default, cms::soa::RangeChecking::Default>;
  SoABlockTemplate()
      : mem_(nullptr),
        elements_(0),
        byteSize_(0),
        x_(nullptr),
        y_(nullptr),
        z_(nullptr),
        t_(nullptr),
        scalar_(nullptr) {}
  SoABlockTemplate(std::byte* mem, size_type elements) : mem_(mem), elements_(elements), byteSize_(0) {
    organizeColumnsFromBuffer();
  }
  SoABlockTemplate(SoABlockTemplate const& _soa_impl_other)
      : mem_(_soa_impl_other.mem_),
        elements_(_soa_impl_other.elements_),
        byteSize_(_soa_impl_other.byteSize_),
        x_{_soa_impl_other.x_},
        y_{_soa_impl_other.y_},
        z_{_soa_impl_other.z_},
        t_{_soa_impl_other.t_},
        scalar_{_soa_impl_other.scalar_} {}
  SoABlockTemplate& operator=(SoABlockTemplate const& _soa_impl_other) {
    mem_ = _soa_impl_other.mem_;
    elements_ = _soa_impl_other.elements_;
    byteSize_ = _soa_impl_other.byteSize_;
    x_ = _soa_impl_other.x_;
    y_ = _soa_impl_other.y_;
    z_ = _soa_impl_other.z_;
    t_ = _soa_impl_other.t_;
    scalar_ = _soa_impl_other.scalar_;
    return *this;
  }
  void deepCopy(ConstView const& view) {
    if (elements_ < view.metadata().size())
      throw std::runtime_error(
          "In "
          "SoABlockTemplate"
          "::deepCopy method: number of elements mismatch ");
    memcpy(this->metadata().addressOf_x(), view.metadata().addressOf_x(), view.metadata().size() * sizeof(float));
    memcpy(this->metadata().addressOf_y(), view.metadata().addressOf_y(), view.metadata().size() * sizeof(float));
    memcpy(this->metadata().addressOf_z(), view.metadata().addressOf_z(), view.metadata().size() * sizeof(float));
    memcpy(this->metadata().addressOf_t(), view.metadata().addressOf_t(), view.metadata().size() * sizeof(float));
    memcpy(this->metadata().addressOf_scalar(), view.metadata().addressOf_scalar(), sizeof(size_t));
  }
  template <typename T>
  void ROOTReadStreamer(T& onfile) {
    memcpy(x_, onfile.x_, sizeof(float) * onfile.elements_);
    memcpy(y_, onfile.y_, sizeof(float) * onfile.elements_);
    memcpy(z_, onfile.z_, sizeof(float) * onfile.elements_);
    memcpy(t_, onfile.t_, sizeof(float) * onfile.elements_);
    memcpy(scalar_, onfile.scalar_, sizeof(size_t));
  }
  void ROOTStreamerCleaner() {
    delete[] x_;
    x_ = nullptr;
    delete[] y_;
    y_ = nullptr;
    delete[] z_;
    z_ = nullptr;
    delete[] t_;
    t_ = nullptr;
    delete[] scalar_;
    scalar_ = nullptr;
  }
  template <typename T>
  friend void dump();

private:
  void organizeColumnsFromBuffer() {
    if constexpr (alignmentEnforcement == cms::soa::AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(mem_) % alignment)
        throw std::runtime_error(
            "In "
            "SoABlockTemplate"
            "::"
            "SoABlockTemplate"
            ": misaligned buffer");
    auto _soa_impl_curMem = mem_;
    x_ = reinterpret_cast<float*>(_soa_impl_curMem);
    _soa_impl_curMem += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(x_) % alignment)
        throw std::runtime_error(
            "In layout constructor: misaligned column: "
            "x");
    y_ = reinterpret_cast<float*>(_soa_impl_curMem);
    _soa_impl_curMem += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(y_) % alignment)
        throw std::runtime_error(
            "In layout constructor: misaligned column: "
            "y");
    z_ = reinterpret_cast<float*>(_soa_impl_curMem);
    _soa_impl_curMem += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(z_) % alignment)
        throw std::runtime_error(
            "In layout constructor: misaligned column: "
            "z");
    t_ = reinterpret_cast<float*>(_soa_impl_curMem);
    _soa_impl_curMem += cms::soa::alignSize(elements_ * sizeof(float), alignment);
    if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(t_) % alignment)
        throw std::runtime_error(
            "In layout constructor: misaligned column: "
            "t");
    scalar_ = reinterpret_cast<size_t*>(_soa_impl_curMem);
    _soa_impl_curMem += cms::soa::alignSize(sizeof(size_t), alignment);
    if constexpr (alignmentEnforcement == AlignmentEnforcement::enforced)
      if (reinterpret_cast<intptr_t>(scalar_) % alignment)
        throw std::runtime_error(
            "In layout constructor: misaligned column: "
            "scalar");
    byteSize_ = computeDataSize(elements_);
    if (mem_ + byteSize_ != _soa_impl_curMem)
      throw std::runtime_error(
          "In "
          "SoABlockTemplate"
          "::"
          "SoABlockTemplate"
          ": unexpected end pointer.");
  }
  static constexpr std::pair<size_type, size_type> computeMethodsNumber() {
    size_type _soa_methods_count = 0;
    size_type _soa_const_methods_count = 0;
    return {_soa_methods_count, _soa_const_methods_count};
  }
  static_assert(computeMethodsNumber().first <= 1,
                "There can be at most one SOA_METHODS macro. Please declare all your methods inside the same macro.");
  static_assert(
      computeMethodsNumber().second <= 1,
      "There can be at most one SOA_CONST_METHODS macro. Please declare all your methods inside the same macro.");
  std::byte* mem_;
  size_type elements_;
  size_type const scalar_ = 1;
  byte_size_type byteSize_;
  float* x_ = nullptr;
  float* y_ = nullptr;
  float* z_ = nullptr;
  float* t_ = nullptr;
  size_t* scalar_ = nullptr;
};
using SoABlock = SoABlockTemplate<>;
static void ____C_A_T_C_H____T_E_S_T____0();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
namespace {
  Catch::AutoReg autoRegistrar1(Catch::makeTestInvoker(&____C_A_T_C_H____T_E_S_T____0),
                                ::Catch::SourceLineInfo("SoABlock.cc", static_cast<std::size_t>(23)),
                                Catch::StringRef(),
                                Catch::NameAndTags{"SoATemplate"});
}
#pragma clang diagnostic pop
static void ____C_A_T_C_H____T_E_S_T____0() {}
