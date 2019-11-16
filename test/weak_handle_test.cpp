// © 2019 Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/weak_handle.h>

#include <cstddef>
#include <iostream>

using namespace jfc;

TEST_CASE( "jfc::weak_handle test", "[jfc::weak_handle]" )
{
    shared_handle<int> shared(1, [&](int){});

    weak_handle<int> weak(shared);

    SECTION("locking successfully copies the still in scope shared_handle")
    {
        const auto new_shared = weak.lock();

        REQUIRE(new_shared);

        REQUIRE(shared.get() == (*new_shared).get());
    }
    
    SECTION("locking a shared_handle that has fallen out of scope produces a null optional & expired is true")
    {
        auto mkshared = []()
        {
            return shared_handle<int>(1, [&](int a){});
        };

        weak_handle<int> b(mkshared());

        REQUIRE(!b.lock());

        REQUIRE(b.expired());
    }

    SECTION("copy semantics work:")
    {
        weak_handle<int> a(shared);

        auto b(a.lock());

        REQUIRE(b == shared);
    }

    SECTION("move semantics")
    {
        auto a = std::move(weak);

        REQUIRE(a.lock());
    }
}

