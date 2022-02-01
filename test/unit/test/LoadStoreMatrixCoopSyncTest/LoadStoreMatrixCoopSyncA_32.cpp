/*******************************************************************************
 *
 * MIT License
 *
 * Copyright 2021 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#include <type_traits>

#include "KernelGenerator.h"
#include "detail/LoadStoreMatrixCoopSync.h"
#include "test/UnitTest.h"

struct TestParams : public UnitTestParams
{
    using Base = UnitTestParams;

    // Types: ALL + double
    // Block Sizes: 32 x BlockK
    // Layouts: N, T
    using Types        = typename Base::TestTypes32x32;
    using BlockSizes   = typename Base::TestBlockSizes32x32;
    using Layouts      = typename Base::TestLayoutsAll;
    using KernelParams = typename CombineLists<Types, BlockSizes, Layouts>::Result;

    // Assemble the kernel generator
    // Kernel: LoadStoreMatrixCoopSyncA
    using GeneratorImpl   = LoadStoreMatrixCoopSyncGeneratorA;
    using KernelGenerator = KernelGenerator<KernelParams, GeneratorImpl>;

    // Sanity check for kernel generator
    static_assert(std::is_same<typename GeneratorImpl::ResultT, typename Base::KernelT>::value,
                  "Kernels from this generator do not match testing interface");

    static inline typename KernelGenerator::ResultT kernels()
    {
        return KernelGenerator::generate();
    }

    static inline std::vector<Base::Param1T> param1s()
    {
        return {0.0, 1.0}; // Split by waves in same rol and col
    }

    static inline std::vector<Base::Param2T> param2s()
    {
        return {
            0.0,
            1.0,
            2.0,
            3.0 // 1 - 4 waves
#ifdef WMMA_EXTENDED_TESTS
            ,
            4.0,
            5.0,
            6.0,
            7.0 // 8 waves
#endif // WMMA_EXTENDED_TESTS
        };
    }
};

// Test suite for unique parameterization
class LoadStoreMatrixSyncCoopATest32 : public UnitTest
{
};

TEST_P(LoadStoreMatrixSyncCoopATest32, RunKernel)
{
    this->RunKernel();
}

INSTANTIATE_TEST_SUITE_P(KernelTests,
                         LoadStoreMatrixSyncCoopATest32,
                         ::testing::Combine(::testing::ValuesIn(TestParams::kernels()),
                                            ::testing::ValuesIn(TestParams::threadBlocks()),
                                            ::testing::ValuesIn(TestParams::problemSizes()),
                                            ::testing::ValuesIn(TestParams::param1s()),
                                            ::testing::ValuesIn(TestParams::param2s())));