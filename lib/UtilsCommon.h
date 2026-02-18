#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace Visca {
class UtilsCommon {
public:
    /**
     * @brief Serialize a specific data stuct type to binary
     *
     * @tparam StructType The specific struct type to serialize
     * @param stuctData The numeric type to serialize
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     * @return std::vector<uint8_t> The serialized struct data in bytes
     */
    template <typename StructType> static std::vector<uint8_t> SerializeStructData(const StructType& structData)
    {
        std::vector<uint8_t> bytes(sizeof(structData));
        std::memcpy(bytes.data(), &structData, sizeof(structData));
        return bytes;
    }

    /**
     * @brief Deserialize data to a specific struct
     *
     * @tparam StructType The specific numeric type to deserialize
     * @param structBytes The data to deserialize
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     * @return StructType The deserialized struct data
     */
    template <typename StructType> static StructType DeserializeStructData(const std::vector<uint8_t>& structBytes)
    {
        StructType data;
        std::memcpy(&data, structBytes.data(), sizeof(data));
        return data;
    }

    /**
     * @brief Serialize a string to binary data
     *
     * @param string The string to serialize
     * @return The serialized string as binary data
     */
    static std::vector<uint8_t> SerializeString(const std::string& string);

    /**
     * @brief Deserialize data to a string
     *
     * @param stringBytes The data to deserialize
     * @return The deserialized dat to as a string
     */
    static std::string DeserializeString(const std::vector<uint8_t>& stringBytes);

    /**
     * @brief Serialize a POD data type
     *
     * See https://docs.microsoft.com/en-us/cpp/cpp/trivial-standard-layout-and-pod-types?view=msvc-160
     * and https://codereview.stackexchange.com/questions/260597/de-serialization-of-structs-representing-tcp-messages
     *
     * @tparam T The POD data type
     * @param podDataType The POD variable
     * @return QList<uint8_t> Serialized POD
     */
    template <typename T> static std::vector<uint8_t> SerializePlainOldDataTypes(const T& podDataType)
    {
        static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value,
            "SerializePlainOldDataTypes requires POD types");

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&podDataType);
        return std::vector<uint8_t>(ptr, ptr + sizeof(T));
    }

    /**
     * @brief Deserialize data to a POD data type
     *
     * @tparam T The POD data type
     * @param data The data to deserialize
     * @return T The deserialized POD data type
     */
    template <typename T> static std::optional<T> DeserializePlainOldDataTypes(const std::vector<uint8_t>& data)
    {
        T podDataType;

        if (data.size() != sizeof podDataType) {
            return std::nullopt;
        }

        if (!(std::is_trivial<T>() && std::is_standard_layout<T>())) {
            return std::nullopt;
        }

        auto ptr = reinterpret_cast<const uint8_t*>(&data[0]);
        std::memcpy(&podDataType, ptr, sizeof podDataType);

        return podDataType;
    }

    /**
     * @brief Check the endianess of the host
     *
     * Let's store 1 into the i field of a HostEndianness union. On a big-endian machine, the four bytes of the c array
     * field will be {0, 0, 0, 1}, but on a little-endian machine, the bytes will be {1, 0, 0, 0}. To determine the
     * machine's endianness, we simply look at the first character in the array -- on big-endian machines it will hold
     * 0; on little-endian machines it will hold 1.
     *
     * See https://www.realtime.bc.ca/articles/endian-safe.html
     *
     */
    union HostEndianness {
        int i = 1;
        char c[sizeof(int)];

        bool isBigEndian() const { return c[0] == 0; }
        bool isLittleEndian() const { return c[0] != 0; }
    };

    /**
     * @brief Swap an argument according to preferred endianess
     *
     * If the system is big-endian (HostEndianness().isBigEndian() is true) and we want big-endian (isBigEndianInMemory
     * is true) then there is no need for swap.
     *
     * The same applies if the system is little-endian (HostEndianness().isBigEndian() is false) and we want
     * little-endian (isBigEndianInMemory is false).
     *
     * On all the other options there will be a swap.
     *
     * Little-endian: LSB to MSB
     * Big-endian: MSB to LSB
     *
     * See https://www.realtime.bc.ca/articles/endian-safe.html
     *
     * @tparam T The type of the argument
     * @param arg The argument to swap.
     * @param preferBigEndianInMemory Indicate which endianness we want, setting to true if we want big-endian byte
     * order in memory.
     * @return T The argument swapped or not
     */
    template <typename T> static T EndianSwap(const T& arg, bool preferBigEndianInMemory)
    {
        if (!std::is_arithmetic<T>::value) {
            return T {};
        }

        if (HostEndianness().isBigEndian() == preferBigEndianInMemory) {
            return arg;
        } else {
            T ret;

            std::memcpy(&ret, &arg, sizeof(T));

            uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&ret);
            std::reverse(byte_ptr, byte_ptr + sizeof(T));

            return ret;
        }
    }

    /**
     * @brief Serialize a specific numeric type to specific endianness
     *
     * @tparam NumericType The specific numeric type to serialize
     * @param numericValueToSerialize The numeric type to serialize
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     * @return std::vector<uint8_t> The serialized numeric type
     */
    template <typename NumericType>
    static std::vector<uint8_t> SerializeNumericValues(NumericType numericValueToSerialize, bool toBigEndian = false)
    {
        if (!(std::is_arithmetic<NumericType>::value)) {
            return std::vector<uint8_t>();
        }

        std::vector<uint8_t> serializedNumericValue(sizeof(NumericType));

        numericValueToSerialize = UtilsCommon::EndianSwap<NumericType>(numericValueToSerialize, toBigEndian);
        std::memcpy(serializedNumericValue.data(), &numericValueToSerialize, sizeof(NumericType));

        return serializedNumericValue;
    }

    /**
     * @brief Serialize a specific numeric type and append it to an existing vector
     *
     * @tparam NumericType The specific numeric type to serialize
     * @param numericValueToSerialize The numeric type to serialize
     * @param outputVector The vector to append the serialized bytes to
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     */
    template <typename NumericType>
    static void SerializeNumericValueAppendToVector(
        NumericType numericValueToSerialize, std::vector<uint8_t>& outputVector, bool toBigEndian = false)
    {
        if (!std::is_arithmetic<NumericType>::value) {
            return;
        }

        numericValueToSerialize = UtilsCommon::EndianSwap<NumericType>(numericValueToSerialize, toBigEndian);

        // Get a pointer to the start of the numeric value's bytes
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&numericValueToSerialize);

        // Calculate the number of bytes to write
        constexpr size_t size = sizeof(NumericType);

        // Append the bytes directly to the vector.
        outputVector.insert(outputVector.end(), bytes, bytes + size);
    }

    /**
     * @brief Deserialize data to a numeric value
     *
     * @tparam NumericType The specific numeric type to deserialize
     * @param data The data to deserialize
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     * @return NumericType The deserialized numeric type
     */
    template <typename NumericType>
    static NumericType DeserializeNumericValues(const std::vector<uint8_t>& data, bool toBigEndian = false)
    {
        // Check if vector is large enough
        if (data.size() < sizeof(NumericType)) {
            return NumericType();
        }

        // Use the iterator version with data.begin() and data.end()
        return DeserializeNumericValues<NumericType>(data.begin(), data.end(), toBigEndian);
    }

    /**
     * @brief Deserialize data to a numeric value using a byte range defined by iterators.
     *
     * @tparam NumericType The specific numeric type to deserialize.
     * @tparam ByteIterator An iterator type that points to uint8_t (or compatible).
     * @param begin The iterator pointing to the first byte of the data chunk.
     * @param end The iterator pointing one past the last byte of the data chunk.
     * @param toBigEndian If true, treat the input data as Big Endian. Default is false (Little Endian).
     * @return NumericType The deserialized numeric type.
     */
    template <typename NumericType, typename ByteIterator>
    static NumericType DeserializeNumericValues(ByteIterator begin, ByteIterator end, bool toBigEndian = false)
    {
        if (!(std::is_arithmetic<NumericType>::value)) {
            return NumericType {};
        }

        size_t data_size = std::distance(begin, end);

        if (data_size < sizeof(NumericType)) {
            return NumericType {};
        }

        NumericType deserializedNumericValue;

        // Create a temporary byte buffer of the required size
        uint8_t buffer[sizeof(NumericType)];

        // Copy the bytes from the iterator range into the buffer
        std::copy(begin, std::next(begin, sizeof(NumericType)), buffer);

        // Copy from the temporary buffer into the target variable
        std::memcpy(&deserializedNumericValue, buffer, sizeof(NumericType));

        deserializedNumericValue = UtilsCommon::EndianSwap<NumericType>(deserializedNumericValue, toBigEndian);

        return deserializedNumericValue;
    }

    /**
     * @brief Convert an unsigned number to a hex string (upper case)
     *
     * @tparam NumericType The specific numeric type to convert to hex string
     * @param number The unsigned number to convert to hex string
     * @param toBigEndian If it's true, then it's Big Endian. If it's false, then it's Little Endian. Default value is
     * false (Little Endian)
     * @return std::string The converted hex string from the unsigned number
     */
    template <typename NumericType>
    static std::string UnsignedNumberToHexString(NumericType number, bool toBigEndian = false)
    {
        std::stringstream stream;

        if (!(std::is_arithmetic<NumericType>::value)) {
            return nullptr;
        }

        if (!(std::is_unsigned<NumericType>::value)) {
            return nullptr;
        }

        number = UtilsCommon::EndianSwap<NumericType>(number, toBigEndian);

        stream << "0x" << std::setfill('0') << std::setw(sizeof(NumericType) * 2) << std::hex << std::uppercase
               << number << std::nouppercase << std::dec;

        return stream.str();
    }

    /**
     * @brief Convert a floating point number to string with a specific decimal precision
     *
     * @tparam NumericType The specific numeric type to convert to string
     * @param number The floating point number to convert to string with a specific precision
     * @param precision The precision of the floating point number
     * @return std::string The converted number to string
     */
    template <typename NumericType>
    static std::string FloatingPointNumberPrecisionToString(NumericType number, uint8_t precision)
    {
        std::stringstream stream;

        if (!(std::is_arithmetic<NumericType>::value)) {
            return nullptr;
        }

        if (!(std::is_floating_point<NumericType>::value)) {
            return nullptr;
        }

        stream << std::fixed << std::setprecision(precision) << number;

        return stream.str();
    }

    /**
     * @brief Convert a byte vector to a hex string
     *
     * This function converts a byte vector to a hex string representation. It iterates
     * through each byte in the vector, converting it to its hexadecimal representation
     * and appending it to the resulting string. The function uses std::ostringstream
     * to format the bytes as hex. The resulting string is in uppercase and does not
     * include any separators between the hex values.
     *
     * @param data The byte vector to convert
     * @return std::string The hex string representation of the byte vector
     */
    static std::string bytesToHex(const std::vector<uint8_t>& data)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (uint8_t byte : data) {
            oss << std::setw(2) << static_cast<int>(byte) << " ";
        }
        return oss.str();
    }

    /**
     * @brief Print a number in hexadecimal format
     *
     * This function prints a numeric value in hexadecimal format. It supports
     * various numeric types (integers, floats, etc.) and shows the hex
     * representation of the underlying bytes.
     *
     * @tparam T The numeric type to print (e.g., int, float, double)
     * @tparam std::enable_if<std::is_arithmetic<T>::value, int>::type
     * @param value The number to print
     * @param prefix Whether to include "0x" prefix (default true)
     */
    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    static std::string toHex(T value, bool prefix = true)
    {
        std::ostringstream oss;
        if (prefix) {
            oss << "0x";
        }

        // Convert to byte array
        uint8_t bytes[sizeof(T)];
        std::memcpy(bytes, &value, sizeof(T));

        // Print bytes in reverse order (little-endian) for consistency with common display
        oss << std::hex << std::setfill('0');
        for (int i = sizeof(T) - 1; i >= 0; --i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }

        return oss.str();
    }

private:
    UtilsCommon();
};
}
