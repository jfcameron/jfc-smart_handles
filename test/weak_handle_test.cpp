// © 2019 Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/weak_handle.h>

#include <cstddef>

using namespace jfc;

TEST_CASE( "jfc::weak_handle test", "[jfc::weak_handle]" )
{
    shared_handle<int> shared(1, [&](int a){});

    weak_handle<int> weak(shared);

    SECTION("constructor work as expected")
    {
        auto new_shared = weak.lock();

        REQUIRE(new_shared);

        REQUIRE(shared.get() == (*new_shared).get());
    }

    REQUIRE(true);
}

