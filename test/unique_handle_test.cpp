// © 2019 Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/unique_handle.h>

#include <cstddef>

namespace
{
    constexpr int INITIALIZED(1);
}

using namespace jfc;

TEST_CASE( "jfc::unique_handle test", "[jfc::unique_handle]" )
{
    unique_handle<int> unique(INITIALIZED, [](const int){});

    SECTION("constructed instance contains a well formed handle")
    {
        REQUIRE(unique.get() == INITIALIZED);
    }

    SECTION("move semantics work. Deletor only called by owner's destructor")
    {
        int deletor_call_count(0);
        
        {
            const auto makeAUnique = [&deletor_call_count]()
            {
                return unique_handle<int>(INITIALIZED, [&deletor_call_count](const int)
                {
                    ++deletor_call_count;
                });
            };

            auto b = makeAUnique();
        }

        REQUIRE(deletor_call_count == 1);
    }
}

