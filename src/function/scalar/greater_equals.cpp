//
// Created by JinHai on 2022/9/28.
//

#include "greater_equals.h"
#include "function/scalar_function.h"
#include "function/scalar_function_set.h"
#include <cmath>

namespace infinity {

struct GreaterEqualsFunction {
    template<typename TA, typename TB, typename TC>
    static inline void
    Run(TA left, TB right, TC& result) {
        result = left >= right;
    }
};

template<>
inline void
GreaterEqualsFunction::Run(VarcharT left, VarcharT right, bool & result) {
    if(left.IsInlined()) {
        if(right.IsInlined()) {
            result = (memcmp(left.prefix, right.prefix, VarcharT::INLINE_LENGTH) >= 0);
            return ;
        }
    } else if(right.IsInlined()) {
        ;
    } else {
        // Both left and right are not inline
        u16 min_len = std::min(right.length, left.length);
        if(memcmp(left.prefix, right.prefix, VarcharT::PREFIX_LENGTH) >= 0) {
            result = (memcmp(left.ptr, right.ptr, min_len) >= 0);
            return ;
        }
    }
    result = false;
}

template<>
inline void
GreaterEqualsFunction::Run(MixedT left, BigIntT right, bool& result) {
    NotImplementError("Not implement: mixed == bigint")
}

template<>
inline void
GreaterEqualsFunction::Run(BigIntT left, MixedT right, bool& result) {
    GreaterEqualsFunction::Run(right, left, result);
}

template<>
inline void
GreaterEqualsFunction::Run(MixedT left, DoubleT right, bool& result) {
    NotImplementError("Not implement: mixed == double")
}

template<>
inline void
GreaterEqualsFunction::Run(DoubleT left, MixedT right, bool& result) {
    GreaterEqualsFunction::Run(right, left, result);
}

template<>
inline void
GreaterEqualsFunction::Run(MixedT left, VarcharT right, bool& result) {
    NotImplementError("Not implement: mixed == varchar")
}

template<>
inline void
GreaterEqualsFunction::Run(VarcharT left, MixedT right, bool& result) {
    GreaterEqualsFunction::Run(right, left, result);
}

template<typename CompareType>
static void
GenerateGreaterEqualsFunction(std::shared_ptr<ScalarFunctionSet>& function_set_ptr, DataType data_type) {

    ScalarFunction greater_equals_function(
            ">=",
            { data_type, data_type },
            { DataType(LogicalType::kBoolean) },
            &ScalarFunction::BinaryFunction<CompareType, CompareType, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(greater_equals_function);
}

void
RegisterGreaterEqualsFunction(const std::unique_ptr<Catalog> &catalog_ptr) {
    std::shared_ptr<ScalarFunctionSet> function_set_ptr = std::make_shared<ScalarFunctionSet>(">=");

    GenerateGreaterEqualsFunction<TinyIntT>(function_set_ptr, DataType(LogicalType::kTinyInt));
    GenerateGreaterEqualsFunction<SmallIntT>(function_set_ptr, DataType(LogicalType::kSmallInt));
    GenerateGreaterEqualsFunction<IntegerT>(function_set_ptr, DataType(LogicalType::kInteger));
    GenerateGreaterEqualsFunction<BigIntT>(function_set_ptr, DataType(LogicalType::kBigInt));
    GenerateGreaterEqualsFunction<HugeIntT>(function_set_ptr, DataType(LogicalType::kHugeInt));
    GenerateGreaterEqualsFunction<FloatT>(function_set_ptr, DataType(LogicalType::kFloat));
    GenerateGreaterEqualsFunction<DoubleT>(function_set_ptr, DataType(LogicalType::kDouble));

//    GenerateGreaterEqualsFunction<Decimal16T>(function_set_ptr, DataType(LogicalType::kDecimal16));
//    GenerateGreaterEqualsFunction<Decimal32T>(function_set_ptr, DataType(LogicalType::kDecimal32));
//    GenerateGreaterEqualsFunction<Decimal64T>(function_set_ptr, DataType(LogicalType::kDecimal64));
//    GenerateGreaterEqualsFunction<Decimal128T>(function_set_ptr, DataType(LogicalType::kDecimal128));

    GenerateGreaterEqualsFunction<VarcharT>(function_set_ptr, DataType(LogicalType::kVarchar));
//    GenerateGreaterEqualsFunction<CharT>(function_set_ptr, DataType(LogicalType::kChar));

//    GenerateGreaterEqualsFunction<DateT>(function_set_ptr, DataType(LogicalType::kDate));
//    GenerateGreaterEqualsFunction<TimeT>(function_set_ptr, DataType(LogicalType::kTime));
//    GenerateGreaterEqualsFunction<DateTimeT>(function_set_ptr, DataType(LogicalType::kDateTime));
//    GenerateGreaterEqualsFunction<TimestampT>(function_set_ptr, DataType(LogicalType::kTimestamp));
//    GenerateGreaterEqualsFunction<TimestampTZT>(function_set_ptr, DataType(LogicalType::kTimestampTZ));

//    GenerateGreaterEqualsFunction<MixedT>(function_set_ptr, DataType(LogicalType::kMixed));

    ScalarFunction mix_greater_equals_bigint(
            ">=",
            { DataType(LogicalType::kMixed), DataType(LogicalType::kBigInt) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<MixedT, BigIntT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(mix_greater_equals_bigint);

    ScalarFunction bigint_greater_equals_mixed(
            ">=",
            { DataType(LogicalType::kBigInt), DataType(LogicalType::kMixed) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<BigIntT, MixedT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(bigint_greater_equals_mixed);

    ScalarFunction mix_greater_equals_double(
            ">=",
            { DataType(LogicalType::kMixed), DataType(LogicalType::kDouble) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<MixedT, DoubleT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(mix_greater_equals_double);

    ScalarFunction double_greater_equals_mixed(
            ">=",
            { DataType(LogicalType::kDouble), DataType(LogicalType::kMixed) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<DoubleT, MixedT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(double_greater_equals_mixed);

    ScalarFunction mix_greater_equals_varchar(
            ">=",
            { DataType(LogicalType::kMixed), DataType(LogicalType::kVarchar) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<MixedT, VarcharT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(mix_greater_equals_varchar);

    ScalarFunction varchar_greater_equals_mixed(
            ">=",
            { DataType(LogicalType::kVarchar), DataType(LogicalType::kMixed) },
            DataType(kBoolean),
            &ScalarFunction::BinaryFunction<VarcharT, MixedT, BooleanT, GreaterEqualsFunction>);
    function_set_ptr->AddFunction(varchar_greater_equals_mixed);

    catalog_ptr->AddFunctionSet(function_set_ptr);
}

}