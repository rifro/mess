#pragma once
#include <cstdint>

namespace Bocari
{
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;

    // Schone casts zonder stotteren
    template <typename T> constexpr u32 U32(T x) { return static_cast<u32>(x); }
    template <typename T> constexpr i32 I32(T x) { return static_cast<i32>(x); }

    // --- Math Helpers ---
    namespace Math {
        template <typename T> constexpr T pow2(T x) { return x * x; }
        template <typename T> constexpr T pow3(T x) { return x * x * x; }
    }
    using Math::pow2; // Alias in Bocari namespace

    namespace PointType {
        enum PointTypeEnum : u8 {
            None        = 0,
            Noise       = 1,
            Chaos       = 2, // Nog aanwezig voor legacy compatibiliteit
            Duplicate   = 3,

            Blind       = 4, // conns.size() < 2
            Cross       = 5,
            Elbow       = 6,
            Pipe        = 7,
            Plane       = 8,
            Reducer     = 9,
            Tee         = 10,

            HypotheticalFlag = 128,
            GapFill          = 7 | 128 // Pipe | HypotheticalFlag
        };
    }

// Eventuele wiskundige types zoals Vec3f kunnen hier ook
    struct Vec3f { float x, y, z; };

}
