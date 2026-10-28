// © Joseph Cameron - MIT License

#if defined(__SANITIZE_THREAD__)
#define JFC_SMART_HANDLES_TEST_UNDER_TSAN
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define JFC_SMART_HANDLES_TEST_UNDER_TSAN
#endif
#endif

#ifndef JFC_SMART_HANDLES_TEST_UNDER_TSAN

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

namespace
{
    thread_local bool g_FailAllocations(false);

    template<typename Make>
    void while_allocations_fail(const Make aMake)
    {
        g_FailAllocations = true;

        try { aMake(); }
        catch (...) { g_FailAllocations = false; throw; }

        g_FailAllocations = false;
    }
}

void *operator new(const std::size_t aSize, const std::nothrow_t &) noexcept
{
    return g_FailAllocations ? nullptr : std::malloc(aSize ? aSize : 1);
}

void *operator new(const std::size_t aSize)
{
    if (void *const p = operator new(aSize, std::nothrow)) return p;

    throw std::bad_alloc();
}

void operator delete(void *const p) noexcept { std::free(p); }

void operator delete(void *const p, std::size_t) noexcept { std::free(p); }

void operator delete(void *const p, const std::nothrow_t &) noexcept { std::free(p); }

using namespace jfc;

TEST_CASE("a handle whose control block cannot be made is deleted, as std::shared_ptr deletes its pointer", "[jfc::shared_handle]")
{
    int deleted(0);

    shared_handle<int>::deleter_type deleter = [&](const int aHandle){ deleted += aHandle; };

    REQUIRE_THROWS_AS(while_allocations_fail([&]{ shared_handle<int>(7, std::move(deleter)); }), std::bad_alloc);
    REQUIRE(deleted == 7);
}

TEST_CASE("a unique_handle whose control block cannot be made keeps its handle", "[jfc::unique_handle][jfc::shared_handle]")
{
    int deleted(0);

    {
        unique_handle<int> unique(7, [&](const int aHandle){ deleted += aHandle; });

        REQUIRE_THROWS_AS(while_allocations_fail([&]{ shared_handle<int>{std::move(unique)}; }), std::bad_alloc);
        REQUIRE(unique);
        REQUIRE(unique.get() == 7);
        REQUIRE(deleted == 0);
    }

    REQUIRE(deleted == 7);
}

#endif
