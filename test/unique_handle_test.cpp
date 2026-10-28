// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/unique_handle.h>

#include <utility>

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
            unique_handle<int> a(INITIALIZED, [&deletor_call_count](const int){ ++deletor_call_count; });

            {
                const auto b = std::move(a);

                REQUIRE(!a);
                REQUIRE(b.get() == INITIALIZED);
                REQUIRE(deletor_call_count == 0);
            }

            REQUIRE(deletor_call_count == 1);
        }

        REQUIRE(deletor_call_count == 1);
    }
}

