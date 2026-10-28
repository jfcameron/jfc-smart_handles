// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>
#include <jfc/types.h>

#include <jfc/weak_handle.h>

#include <utility>

using namespace jfc;

TEST_CASE( "jfc::weak_handle test", "[jfc::weak_handle]" )
{
    shared_handle<int> shared(1, [&](int){});

    weak_handle<int> weak(shared);

    SECTION("locking successfully copies the still in scope shared_handle")
    {
        const auto new_shared = weak.lock();

        REQUIRE(new_shared);

        REQUIRE(shared.get() == new_shared.get());
    }
    
    SECTION("locking a shared_handle that has fallen out of scope produces an empty shared_handle & expired is true")
    {
        auto mkshared = []()
        {
            return shared_handle<int>(1, [](int){});
        };

        weak_handle<int> b(mkshared());

        REQUIRE(!b.lock());

        REQUIRE(b.expired());
    }

    SECTION("copy semantics work, and copies own nothing")
    {
        const weak_handle<int> a(shared);
        const weak_handle<int> b(a);

        weak_handle<int> c;

        c = a;

        REQUIRE(b.lock() == shared);
        REQUIRE(c.lock() == shared);
        REQUIRE(shared.use_count() == 1);
    }

    SECTION("move semantics")
    {
        auto a = std::move(weak);

        REQUIRE(a.lock());
    }
}


TEST_CASE("jfc::weak_handle lock gives a shared_handle, empty once the handle is deleted", "[jfc::weak_handle]")
{
    SECTION("a share, counted among the others")
    {
        const shared_handle<int> owner(3, [](const int){});
        const weak_handle<int> weak(owner);

        const shared_handle<int> locked = weak.lock();

        REQUIRE(locked == owner);
        REQUIRE(owner.use_count() == 2);
    }

    SECTION("empty, once the last share is gone")
    {
        int calls(0);

        shared_handle<int> owner(3, [&](const int){ ++calls; });
        auto other = owner;
        const weak_handle<int> weak(owner);

        owner.reset();

        REQUIRE(!weak.expired());

        other.reset();

        const shared_handle<int> locked = weak.lock();

        REQUIRE(calls == 1);
        REQUIRE(weak.expired());
        REQUIRE(!locked);
        REQUIRE(locked.get() == 0);
        REQUIRE(locked.use_count() == 0);
    }

    SECTION("empty, from an empty shared_handle")
    {
        const weak_handle<int> weak{shared_handle<int>()};

        REQUIRE(weak.expired());
        REQUIRE(!weak.lock());
    }
}
