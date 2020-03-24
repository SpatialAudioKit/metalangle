//
// Copyright 2019 The ANGLE Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// common.h: Common header for other metal source code.

#ifndef LIBANGLE_RENDERER_METAL_SHADERS_COMMON_H_
#define LIBANGLE_RENDERER_METAL_SHADERS_COMMON_H_

#ifndef GENERATE_SOURCE_STRING
#    include <simd/simd.h>
#    include <metal_stdlib>
#endif

#include "constants.h"

#define ANGLE_KERNEL_GUARD(IDX, MAX_COUNT) \
    if (IDX >= MAX_COUNT)                  \
    {                                      \
        return;                            \
    }

using namespace metal;

// Common constant defined number of color outputs
constant uint32_t kNumColorOutputs [[function_constant(0)]];
constant bool kColorOutputAvailable0 = kNumColorOutputs > 0;
constant bool kColorOutputAvailable1 = kNumColorOutputs > 1;
constant bool kColorOutputAvailable2 = kNumColorOutputs > 2;
constant bool kColorOutputAvailable3 = kNumColorOutputs > 3;
constant bool kColorOutputAvailable4 = kNumColorOutputs > 4;
constant bool kColorOutputAvailable5 = kNumColorOutputs > 5;
constant bool kColorOutputAvailable6 = kNumColorOutputs > 6;
constant bool kColorOutputAvailable7 = kNumColorOutputs > 7;

namespace rx
{
namespace mtl_shader
{

// Full screen triangle's vertices
constant float2 gCorners[3] = {float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f)};

struct MultipleColorOutputs
{
    float4 color0 [[color(0), function_constant(kColorOutputAvailable0)]];
    float4 color1 [[color(1), function_constant(kColorOutputAvailable1)]];
    float4 color2 [[color(2), function_constant(kColorOutputAvailable2)]];
    float4 color3 [[color(3), function_constant(kColorOutputAvailable3)]];
    float4 color4 [[color(4), function_constant(kColorOutputAvailable4)]];
    float4 color5 [[color(5), function_constant(kColorOutputAvailable5)]];
    float4 color6 [[color(6), function_constant(kColorOutputAvailable6)]];
    float4 color7 [[color(7), function_constant(kColorOutputAvailable7)]];
};

#define ANGLE_ASSIGN_COLOR_OUPUT(STRUCT_VARIABLE, COLOR_INDEX, VALUE) \
    do                                                                \
    {                                                                 \
        if (kColorOutputAvailable##COLOR_INDEX)                       \
        {                                                             \
            STRUCT_VARIABLE.color##COLOR_INDEX = VALUE;               \
        }                                                             \
    } while (0)

static inline MultipleColorOutputs toMultipleColorOutputs(float4 color)
{
    MultipleColorOutputs re;

    ANGLE_ASSIGN_COLOR_OUPUT(re, 0, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 1, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 2, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 3, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 4, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 5, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 6, color);
    ANGLE_ASSIGN_COLOR_OUPUT(re, 7, color);

    return re;
}

static inline float3 cubeTexcoords(float2 texcoords, int face)
{
    texcoords = 2.0 * texcoords - 1.0;
    switch (face)
    {
        case 0:
            return float3(1.0, -texcoords.y, -texcoords.x);
        case 1:
            return float3(-1.0, -texcoords.y, texcoords.x);
        case 2:
            return float3(texcoords.x, 1.0, texcoords.y);
        case 3:
            return float3(texcoords.x, -1.0, -texcoords.y);
        case 4:
            return float3(texcoords.x, -texcoords.y, 1.0);
        case 5:
            return float3(-texcoords.x, -texcoords.y, -1.0);
    }
    return float3(texcoords, 0);
}

template <typename Short>
static inline Short bytesToShort(constant uchar *input, uint offset)
{
    Short inputLo = input[offset];
    Short inputHi = input[offset + 1];
    // Little endian conversion:
    return inputLo | (inputHi << 8);
}

template <typename Int>
static inline uint bytesToInt(constant uchar *input, uint offset)
{
    uint input0 = input[offset];
    uint input1 = input[offset + 1];
    uint input2 = input[offset + 2];
    uint input3 = input[offset + 3];
    // Little endian conversion:
    return as_type<Int>(input0 | (input1 << 8) | (input2 << 16) | (input3 << 24));
}

template <typename Short>
static inline void shortToBytes(Short val, uint offset, device uchar *output)
{
    ushort valUnsigned = as_type<ushort>(val);
    output[offset]     = valUnsigned & 0xff;
    output[offset + 1] = (valUnsigned >> 8) & 0xff;
}

template <typename Int>
static inline void intToBytes(Int val, uint offset, device uchar *output)
{
    uint valUnsigned   = as_type<uint>(val);
    output[offset]     = valUnsigned & 0xff;
    output[offset + 1] = (valUnsigned >> 8) & 0xff;
    output[offset + 2] = (valUnsigned >> 16) & 0xff;
    output[offset + 3] = (valUnsigned >> 24) & 0xff;
}

static inline void int24bitToBytes(uint val, uint offset, device uchar *output)
{
    output[offset]     = val & 0xff;
    output[offset + 1] = (val >> 8) & 0xff;
    output[offset + 2] = (val >> 16) & 0xff;
}

template <unsigned int inputBitCount, unsigned int inputBitStart, typename T>
static inline T getShiftedData(T input)
{
    static_assert(inputBitCount + inputBitStart <= (sizeof(T) * 8),
                  "T must have at least as many bits as inputBitCount + inputBitStart.");
    const T mask = (1 << inputBitCount) - 1;
    return (input >> inputBitStart) & mask;
}

template <unsigned int inputBitCount, unsigned int inputBitStart, typename T>
static inline T shiftData(T input)
{
    static_assert(inputBitCount + inputBitStart <= (sizeof(T) * 8),
                  "T must have at least as many bits as inputBitCount + inputBitStart.");
    const T mask = (1 << inputBitCount) - 1;
    return (input & mask) << inputBitStart;
}

template <unsigned int inputBitCount, typename T>
static inline float normalizedToFloat(T input)
{
    static_assert(inputBitCount <= (sizeof(T) * 8),
                  "T must have more bits than or same bits as inputBitCount.");
    static_assert(inputBitCount <= 23, "Only single precision is supported");

    constexpr float inverseMax = 1.0f / ((1 << inputBitCount) - 1);
    return input * inverseMax;
}

template <typename T>
static inline float normalizedToFloat(T input)
{
    return normalizedToFloat<sizeof(T) * 8, T>(input);
}

template <unsigned int outputBitCount, typename T>
static inline T floatToNormalized(float input)
{
    static_assert(outputBitCount <= (sizeof(T) * 8),
                  "T must have more bits than or same bits as inputBitCount.");
    static_assert(outputBitCount <= 23, "Only single precision is supported");

    return static_cast<T>(((1 << outputBitCount) - 1) * input + 0.5f);
}

template <typename T>
static inline T floatToNormalized(float input)
{
    return floatToNormalized<sizeof(T) * 8, T>(input);
}

}  // namespace mtl_shader
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_SHADERS_COMMON_H_ */
