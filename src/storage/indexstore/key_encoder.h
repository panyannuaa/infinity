#pragma once

#include "common/types/data_type.h"

#include <leveldb/status.h>
#include <string>
#include <type_traits>

namespace infinity {
using Slice = leveldb::Slice;


template <class T>
static T LoadUnaligned(const void* p) {
    T x;
    memcpy(&x, p, sizeof(T));
    return x;
}

// Extracts an 8-byte order-preserving prefix of a given key. Little endian is assumed
template <class Slice>
static uint64_t ExtractHead(const Slice& key) {
    switch (key.size()) {
    case 0:
        return 0;
    case 1:
        return static_cast<uint64_t>(key.data()[0]) << 56;
    case 2:
        return static_cast<uint64_t>(
                   __builtin_bswap16(LoadUnaligned<uint16_t>(key.data())))
               << 48;
    case 3:
        return (static_cast<uint64_t>(
                    __builtin_bswap16(LoadUnaligned<uint16_t>(key.data())))
                << 48) |
               (static_cast<uint64_t>(key.data()[2]) << 40);
    case 4:
        return static_cast<uint64_t>(
                   __builtin_bswap32(LoadUnaligned<uint32_t>(key.data())))
               << 32;

    case 5:
        return (static_cast<uint64_t>(
                    __builtin_bswap32(LoadUnaligned<uint32_t>(key.data())))
                << 32) |
               (static_cast<uint64_t>(key.data()[4]) << 24);

    case 6:
        return (static_cast<uint64_t>(
                    __builtin_bswap32(LoadUnaligned<uint32_t>(key.data())))
                << 32) |
               (static_cast<uint64_t>(
                    __builtin_bswap16(LoadUnaligned<uint16_t>(key.data() + 4)))
                << 16);
    case 7:
        return (static_cast<uint64_t>(
                    __builtin_bswap32(LoadUnaligned<uint32_t>(key.data())))
                << 32) |
               (static_cast<uint64_t>(
                    __builtin_bswap16(LoadUnaligned<uint16_t>(key.data() + 4)))
                << 16) |
               (static_cast<uint64_t>(key.data()[6]) << 8);
    default:
        return __builtin_bswap64(LoadUnaligned<uint64_t>(key.data()));
    }
}

// Order-preserving binary encoding for a particular type so that
// the values could be compared by memcpy their encoded bytes.
// Big-endian not supported right now

typedef void (*EncodeFunc)(const void* value, std::string& buf);

class KeyEncoder {
public:
    template<typename KeyEncoderTraits>
    KeyEncoder(KeyEncoderTraits traits);

    void Encode(const void* value, std::string& buf) const {
        encode_func_(value, buf);
    }
private:
    EncodeFunc encode_func_;
};

extern const KeyEncoder* GetKeyEncoder(LogicalType type);

template <auto T>
struct IsIntegral : public std::false_type {};
template <>
struct IsIntegral<LogicalType::kBoolean> : public std::true_type {};
template <>
struct IsIntegral<LogicalType::kTinyInt> : public std::true_type {};
template <>
struct IsIntegral<LogicalType::kInteger> : public std::true_type {};
template <>
struct IsIntegral<LogicalType::kBigInt> : public std::true_type {};
template <>
struct IsIntegral<LogicalType::kHugeInt> : public std::true_type {};


template<LogicalType data_type, typename Enable = void>
class KeyEncoderTraits {};

/*
template<LogicalType data_type>
class KeyEncoderTraits<data_type, typename std::enable_if<IsIntegral<data_type>::value>::type> {
public:
    static void Encode(const void* value, std::string& buf) {

    }
};
*/
template<>
class KeyEncoderTraits<LogicalType::kBoolean> {
public:
    static void Encode(const void* value, std::string& buf) {
        bool val;
        memcpy(&val, value, sizeof(bool));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kTinyInt> {
public:
    static void Encode(const void* value, std::string& buf) {
        i16 val;
        memcpy(&val, value, sizeof(i16));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kInteger> {
public:
    static void Encode(const void* value, std::string& buf) {
        i32 val;
        memcpy(&val, value, sizeof(i32));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kBigInt> {
public:
    static void Encode(const void* value, std::string& buf) {
        i64 val;
        memcpy(&val, value, sizeof(i64));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kHugeInt> {
public:
    static void Encode(const void* value, std::string& buf) {
        HugeInt val;
        memcpy(&val, value, sizeof(HugeInt));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kDecimal> {
public:
    static void Encode(const void* value, std::string& buf) {
        DecimalType decimal_val;
        memcpy(&decimal_val, value, sizeof(DecimalType));
        buf.append((char*)&decimal_val, sizeof(decimal_val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kDate> {
public:
    static void Encode(const void* value, std::string& buf) {
        DateType val;
        memcpy(&val, value, sizeof(DateType));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kTime> {
public:
    static void Encode(const void* value, std::string& buf) {
        TimeType val;
        memcpy(&val, value, sizeof(TimeType));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kDateTime> {
public:
    static void Encode(const void* value, std::string& buf) {
        DateTimeType val;
        memcpy(&val, value, sizeof(DateTimeType));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kTimestamp> {
public:
    static void Encode(const void* value, std::string& buf) {
        TimestampType val;
        memcpy(&val, value, sizeof(TimestampType));
        buf.append((char*)&val, sizeof(val));
    }
};

template<>
class KeyEncoderTraits<LogicalType::kVarchar> {
public:
    static void Encode(const void* value, std::string& buf) {
        auto slice = reinterpret_cast<const Slice*>(value);
        buf.append(slice->data(), slice->size());
    }
};


}