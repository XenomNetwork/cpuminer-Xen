#if !defined(SIMD_UTILS_H__)
#define SIMD_UTILS_H__ 1

//////////////////////////////////////////////////////////////////////
//
//             SIMD utilities
//
//    Not to be confused with the hashing function of the same name. This
//    is about Single Instruction Multiple Data programming using CPU
//    features such as SSE and AVX.
//
//    This header is the entry point to a suite of macros and functions
//    to perform basic operations on vectors that are useful in crypto
//    mining. Some of these functions have native CPU support for scalar
//    data but not for vectors. The main categories are bit rotation
//    and endian byte swapping
//
//    An attempt was made to make the names as similar as possible to
//    Intel's intrinsic function format. Most variations are to avoid
//    confusion with actual Intel intrinsics, brevity, and clarity.
//
//    This suite supports some operations on regular 64 bit integers
//    as well as 128 bit integers available on recent versions of Linux
//    and GCC.
//
//    It also supports various vector sizes on CPUs that meet the minimum
//    requirements.
//
//    The minimum for any real work is a 64 bit CPU with SSE2,
//    ie an the Intel Core 2.
//
//    Following are the minimum requirements for each vector size. There
//    is no significant 64 bit vectorization therefore SSE2 is the practical
//    minimum for using this code.
//
//    MMX:     64 bit vectors  
//    SSE2:   128 bit vectors  (64 bit CPUs only, such as Intel Core2.
//    AVX2:   256 bit vectors  (Starting with Intel Haswell and AMD Ryzen)
//    AVX512: 512 bit vectors  (still under development)
//
//    Most functions are avalaible at the stated levels but in rare cases
//    a higher level feature may be required with no compatible alternative.
//    Some SSE2 functions have versions optimized for higher feature levels
//    such as SSSE3 or SSE4.1 that will be used automatically on capable
//    CPUs.
//
//    The vector size boundaries are respected to maintain compatibility.
//    For example, an instruction introduced with AVX2 may improve 128 bit
//    vector performance but will not be implemented. A CPU with AVX2 will
//    tend to use 256 bit vectors. On a practical level AVX512 does introduce
//    bit rotation instructions for 128 and 256 bit vectors in addition to
//    its own 5a12 bit vectors. These will not be back ported to replace the
//    SW implementations for the smaller vectors. This policy may be reviewed
//    in the future once AVX512 is established. 
//
//    Strict alignment of data is required: 16 bytes for 128 bit vectors,
//    32 bytes for 256 bit vectors and 64 bytes for 512 bit vectors. 64 byte
//    alignment is recommended in all cases for best cache alignment.
//
//    Windows has problems with function vector arguments larger than
//    128 bits. Stack alignment is only guaranteed to 16 bytes. Always use
//    pointers for larger vectors in function arguments. Macros can be
//    used for larger value arguments.
//
//    An attempt was made to make the names as similar as possible to
//    Intel's intrinsic function format. Most variations are to avoid
//    confusion with actual Intel intrinsics, brevity, and clarity
//
//    The main differences are:
//
//   - the leading underscore(s) "_" and the "i" are dropped from the
//     prefix of vector instructions.
//   - "mm64" and "mm128" used for 64 and 128 bit prefix respectively
//     to avoid the ambiguity of "mm".
//   - the element size does not include additional type specifiers
//      like "epi".
//   - some macros contain value args that are updated.
//   - specialized shift and rotate functions that move elements around
//     use the notation "1x32" to indicate the distance moved as units of
//     the element size.
//   - there is a subset of some functions for scalar data. They may have
//     no prefix nor vec-size, just one size, the size of the data.
//   - Some integer functions are also defined which use a similar notation.
//   
//    Function names follow this pattern:
//
//         prefix_op[esize]_[vsize]
//
//    Prefix: usually the size of the largest vectors used. Following
//            are some examples:
//
//    u64:  unsigned 64 bit integer function
//    i128: signed 128 bit integer function (rarely used)
//    m128: 128 bit vector identifier
//    mm128: 128 bit vector function
//
//    op: describes the operation of the function or names the data
//        identifier.
//
//    esize: optional, element size of operation
//
//    vsize: optional, lane size used when a function operates on elements
//           of vectors within lanes of a vector.
//
//    Ex: mm256_ror1x64_128 rotates each 128 bit lane of a 256 bit vector
//        right by 64 bits.
//
//   Some random thoughts about macros and inline functions, the pros and
//   cons, when to use them, etc:
//
// Macros are very convenient and efficient for statement functions.
// Macro args are passed by value and modifications are seen by the caller.
// Macros should not generally call regular functions unless it is for a
// special purpose such overloading a function name.
// Statement function macros that return a value should not end in ";"
// Statement function macros that return a value and don't modify input args
// may be used in function arguments and expressions.
// Macro args used in expressions should be protected ex: (x)+1
// Macros force inlining, function inlining can be overridden by the compiler.
// Inline functions are preferred when multiple statements or local variables
// are needed.
// The compiler can't do any syntax checking or type checking of args making
// macros difficult to debug.
// Although it is technically posssible to access the callers data without
// they being passed as arguments it is good practice to always define
// arguments even if they have the same name.
//
// General guidelines for inline functions:
//
// Inline functions should not have loops, it defeats the purpose of inlining.
// Inline functions should be short, the benefit is lost and the memory cost
// increases if the function is referenced often.
// Inline functions may call other functions, inlined or not. It is convenient
// for wrapper functions whether or not the wrapped function is itself inlined.
// Care should be taken when unrolling loops that contain calls to inlined
// functions that may be large.
// Large code blocks used only once may use function inlining to
// improve high level code readability without the penalty of function
// overhead.
//
// A major restructuring is taking place shifting the focus from pointers
// to registers. Previously pointer casting used memory to provide transparency
// leaving it up to the compiler to manage everything and it does a very good
// job. The focus has shifted to register arguments for more control
// over the actual instructions assuming the data is in a register and the
// the compiler just needs to manage the registers.
//
// Rather than use pointers to provide type transparency
// specific instructions are used to access specific data as specific types.
// Previously pointers were cast and the compiler was left to find a way
// to get the data from wherever it happened to be to the correct registers.
//
// The utilities defined here make use features like register aliasing
// to optimize operations. Many operations have specialized versions as
// well as more generic versions. It is preferable to use a specialized
// version whenever possible a sthey can take advantage of certain
// optimizations not available to the generic version. Specically the generic
// version usually has a second argument used is some extra calculations.
// 
///////////////////////////////////////////////////////

#include <inttypes.h>
#include <x86intrin.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>

// Various types and overlays
#include "simd-utils/simd-types.h"

// 64 and 128 bit integers.
#include "simd-utils/simd-int.h"

#if defined(__MMX__)

// 64 bit vectors
#include "simd-utils/simd-mmx.h"
#include "simd-utils/intrlv-mmx.h"

#if defined(__SSE2__)

// 128 bit vectors
#include "simd-utils/simd-sse2.h"
#include "simd-utils/intrlv-sse2.h"

#if defined(__AVX__)

// 256 bit vector basics
#include "simd-utils/simd-avx.h"
#include "simd-utils/intrlv-avx.h"

#if defined(__AVX2__)

// 256 bit everything else
#include "simd-utils/simd-avx2.h"
#include "simd-utils/intrlv-avx2.h"

// Skylake-X has all these
#if defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)

// 512 bit vectors
#include "simd-utils/simd-avx512.h"
#include "simd-utils/intrlv-avx512.h"

#endif  // MMX
#endif  // SSE2
#endif  // AVX
#endif  // AVX2
#endif  // AVX512

// Picks implementation based on available CPU features.
#include "simd-utils/intrlv-selector.h"

#endif  // SIMD_UTILS_H__
