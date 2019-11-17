// © 2019 Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>

#include <cstddef>

using namespace jfc;

using GLint = int;
using GLfloat = float;

static constexpr GLint INITIALIZED(1);

static GLint glGenBuffer()
{
    return INITIALIZED;
}

TEST_CASE("jfc::shared_handle", "[jfc::shared_handle]")
{
    bool deleterCalled(false);

    auto glDeleteBuffer = [&](GLint)
    {
        deleterCalled = true;    
    };

    shared_handle<GLint> shared(glGenBuffer(), [&](GLint a)
    {
        glDeleteBuffer(a);
    });
    
    REQUIRE(!deleterCalled);

    SECTION("constructed instance contains a well formed handle")
    {
        REQUIRE(shared.get() == INITIALIZED);
    }

    SECTION("Copy semantics work as expected, equality semantics too")
    {
        const auto two = shared;
        {
            REQUIRE(shared == two);

            const decltype(two) three(two);

            REQUIRE(two == three);

            const decltype(two) four(glGenBuffer(), [&](GLint){});

            REQUIRE(four != three);

            REQUIRE(two.use_count() == 3);
        }

        REQUIRE(!deleterCalled);
    }

    SECTION("shared move semantics")
    {
        REQUIRE(!deleterCalled);

        {
            auto moved = std::move(shared);

            REQUIRE(moved.get() == INITIALIZED);
        }

        REQUIRE(deleterCalled);
    }

    SECTION("from unique move semantics move value and ownership (deletor is not called when moved unique falls out of scope")
    {
        int deleter_call_count(0);

        shared_handle<int> share = unique_handle<int>(INITIALIZED, [&deleter_call_count](const int){deleter_call_count++;});

        REQUIRE(share.get() == INITIALIZED);

        REQUIRE(deleter_call_count == 0);
    }
}

