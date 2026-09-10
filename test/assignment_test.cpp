// © Joseph Cameron - All Rights Reserved

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>
#include <jfc/weak_handle.h>

#include <stdexcept>
#include <utility>

using namespace jfc;

TEST_CASE("jfc::shared_handle assignment", "[jfc::shared_handle]")
{
    int freed_a(0), freed_b(0);

    SECTION("copy assignment releases the resource this instance held, and adopts the other")
    {
        {
            shared_handle<int> a(1, [&](const int){ ++freed_a; });

            {
                shared_handle<int> b(2, [&](const int){ ++freed_b; });

                a = b;

                REQUIRE(freed_a == 1);
                REQUIRE(freed_b == 0);
                REQUIRE(a.get() == 2);
                REQUIRE(a.use_count() == 2);
            }

            REQUIRE(freed_b == 0);
        }

        REQUIRE(freed_b == 1);
    }

    SECTION("move assignment releases the resource this instance held")
    {
        {
            shared_handle<int> a(1, [&](const int){ ++freed_a; });
            shared_handle<int> b(2, [&](const int){ ++freed_b; });

            a = std::move(b);

            REQUIRE(freed_a == 1);
            REQUIRE(a.get() == 2);
            REQUIRE(a.use_count() == 1);
        }

        REQUIRE(freed_b == 1);
    }

    SECTION("assignment from a unique_handle takes its ownership")
    {
        int freed_unique(0);

        {
            shared_handle<int> a(1, [&](const int){ ++freed_a; });

            a = unique_handle<int>(3, [&](const int){ ++freed_unique; });

            REQUIRE(freed_a == 1);
            REQUIRE(freed_unique == 0);
            REQUIRE(a.get() == 3);
        }

        REQUIRE(freed_unique == 1);
    }

    SECTION("a handle held in a variable can be shared without moving an integer")
    {
        int handle(7);

        shared_handle<int> a(handle, [&](const int){ ++freed_a; });

        REQUIRE(a.get() == 7);
    }
}

TEST_CASE("jfc::weak_handle assignment", "[jfc::weak_handle]")
{
    shared_handle<int> first(1, [](const int){});
    shared_handle<int> second(2, [](const int){});

    SECTION("copy assignment observes what the other observes")
    {
        weak_handle<int> a(first), b(second);

        a = b;

        const auto locked = a.lock();

        REQUIRE(locked);
        REQUIRE(locked->get() == 2);
    }

    SECTION("move assignment observes what the other observed")
    {
        weak_handle<int> a(first), b(second);

        a = std::move(b);

        const auto locked = a.lock();

        REQUIRE(locked);
        REQUIRE(locked->get() == 2);
    }

    SECTION("assignment from a shared_handle observes it without owning it")
    {
        weak_handle<int> a(first);

        a = second;

        REQUIRE(!a.expired());

        const auto locked = a.lock();

        REQUIRE(locked);
        REQUIRE(locked->get() == 2);
        REQUIRE(second.use_count() == 2);
    }
}

TEST_CASE("a deleter that does nothing is refused where it is supplied", "[jfc::shared_handle]")
{
    using deleter = shared_handle<int>::deleter_type;

    REQUIRE_THROWS_AS(unique_handle<int>(1, deleter()), std::invalid_argument);
    REQUIRE_THROWS_AS(shared_handle<int>(1, deleter()), std::invalid_argument);
}

TEST_CASE("the deleter runs exactly once however many co-owners there were", "[jfc::shared_handle]")
{
    int calls(0);

    {
        shared_handle<int> a(1, [&](const int){ ++calls; });

        const auto b = a;
        const auto c = b;
        const auto d = c;

        REQUIRE(a.use_count() == 4);
        REQUIRE(calls == 0);
    }

    REQUIRE(calls == 1);
}

TEST_CASE("a moved-from handle is empty, and says so", "[jfc::unique_handle][jfc::shared_handle][jfc::weak_handle]")
{
    SECTION("unique_handle")
    {
        unique_handle<int> source(42, [](const int){});
        const unique_handle<int> destination(std::move(source));

        REQUIRE(source.get() == 0);
        REQUIRE(!source);
        REQUIRE(destination);
        REQUIRE(destination.get() == 42);
        REQUIRE(!(source == destination));
    }

    SECTION("unique_handle, move assigned")
    {
        unique_handle<int> source(42, [](const int){});
        unique_handle<int> destination;

        destination = std::move(source);

        REQUIRE(source.get() == 0);
        REQUIRE(!source);
        REQUIRE(destination.get() == 42);
    }

    SECTION("shared_handle")
    {
        shared_handle<int> source(42, [](const int){});
        const shared_handle<int> destination(std::move(source));

        REQUIRE(source.get() == 0);
        REQUIRE(!source);
        REQUIRE(source.use_count() == 0);
        REQUIRE(destination.get() == 42);
        REQUIRE(destination.use_count() == 1);
    }

    SECTION("weak_handle")
    {
        const shared_handle<int> owner(42, [](const int){});

        weak_handle<int> source(owner);
        const weak_handle<int> destination(std::move(source));

        REQUIRE(source.expired());
        REQUIRE(!source.lock());
        REQUIRE(!destination.expired());
        REQUIRE(destination.lock()->get() == 42);
    }
}

TEST_CASE("a default constructed handle owns nothing and deletes nothing", "[jfc::unique_handle][jfc::shared_handle][jfc::weak_handle]")
{
    SECTION("unique_handle")
    {
        const unique_handle<int> empty;

        REQUIRE(!empty);
        REQUIRE(empty.get() == 0);
    }

    SECTION("shared_handle")
    {
        const shared_handle<int> empty;

        REQUIRE(!empty);
        REQUIRE(empty.get() == 0);
        REQUIRE(empty.use_count() == 0);
    }

    SECTION("weak_handle")
    {
        const weak_handle<int> empty;

        REQUIRE(empty.expired());
        REQUIRE(!empty.lock());
    }

    SECTION("an empty handle can be assigned a real one")
    {
        int calls(0);

        {
            shared_handle<int> target;

            REQUIRE(!target);

            target = shared_handle<int>(7, [&](const int){ ++calls; });

            REQUIRE(target);
            REQUIRE(target.get() == 7);
        }

        REQUIRE(calls == 1);
    }
}

TEST_CASE("reset releases what is held", "[jfc::unique_handle][jfc::shared_handle][jfc::weak_handle]")
{
    SECTION("unique_handle::reset() deletes and empties")
    {
        int calls(0);

        unique_handle<int> u(1, [&](const int){ ++calls; });

        u.reset();

        REQUIRE(calls == 1);
        REQUIRE(!u);
        REQUIRE(u.get() == 0);
    }

    SECTION("unique_handle::reset(handle, deleter) deletes the old and adopts the new")
    {
        int old_calls(0), new_calls(0);

        {
            unique_handle<int> u(1, [&](const int){ ++old_calls; });

            u.reset(2, [&](const int){ ++new_calls; });

            REQUIRE(old_calls == 1);
            REQUIRE(new_calls == 0);
            REQUIRE(u.get() == 2);
        }

        REQUIRE(new_calls == 1);
    }

    SECTION("shared_handle::reset() gives up only this instance's share")
    {
        int calls(0);

        {
            shared_handle<int> a(1, [&](const int){ ++calls; });
            const auto b = a;

            a.reset();

            REQUIRE(calls == 0);
            REQUIRE(!a);
            REQUIRE(b.use_count() == 1);
        }

        REQUIRE(calls == 1);
    }

    SECTION("weak_handle::reset() stops observing")
    {
        const shared_handle<int> owner(1, [](const int){});

        weak_handle<int> w(owner);

        REQUIRE(!w.expired());

        w.reset();

        REQUIRE(w.expired());
        REQUIRE(!w.lock());
    }
}
