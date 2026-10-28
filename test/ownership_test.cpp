// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace jfc;

TEST_CASE("an empty unique_handle becomes an empty shared_handle", "[jfc::unique_handle][jfc::shared_handle]")
{
    std::vector<int> deleted;

    SECTION("a default constructed one")
    {
        const shared_handle<int> shared{unique_handle<int>()};

        REQUIRE(!shared);
        REQUIRE(shared.get() == 0);
        REQUIRE(shared.use_count() == 0);
    }

    SECTION("a moved-from one, which leaves the deleter to the handle it moved to")
    {
        {
            unique_handle<int> source(5, [&](const int aHandle){ deleted.push_back(aHandle); });
            const unique_handle<int> destination(std::move(source));

            const shared_handle<int> shared(std::move(source));

            REQUIRE(!shared);
        }

        REQUIRE(deleted == std::vector<int>{5});
    }

    SECTION("assigned, releasing what the shared_handle held")
    {
        shared_handle<int> shared(1, [&](const int aHandle){ deleted.push_back(aHandle); });

        shared = unique_handle<int>();

        REQUIRE(!shared);
        REQUIRE(deleted == std::vector<int>{1});
    }
}

TEST_CASE("a unique_handle made shared is left empty, and its handle is deleted once", "[jfc::unique_handle][jfc::shared_handle]")
{
    std::vector<int> deleted;

    {
        unique_handle<int> unique(9, [&](const int aHandle){ deleted.push_back(aHandle); });

        {
            const shared_handle<int> shared(std::move(unique));

            REQUIRE(!unique);
            REQUIRE(unique.get() == 0);
            REQUIRE(shared.get() == 9);
            REQUIRE(deleted.empty());
        }

        REQUIRE(deleted == std::vector<int>{9});
    }

    REQUIRE(deleted == std::vector<int>{9});
}

TEST_CASE("a moved-from unique_handle lets go of its deleter", "[jfc::unique_handle]")
{
    const auto captured = std::make_shared<int>(0);

    SECTION("moved")
    {
        unique_handle<int> source(1, [captured](const int){});

        {
            const unique_handle<int> destination(std::move(source));
        }

        REQUIRE(captured.use_count() == 1);
    }

    SECTION("move assigned")
    {
        unique_handle<int> source(1, [captured](const int){});

        {
            unique_handle<int> destination;

            destination = std::move(source);
        }

        REQUIRE(captured.use_count() == 1);
    }
}

TEST_CASE("a deleter that throws is called once", "[jfc::unique_handle]")
{
    int calls(0);

    const auto throwing = [&](const int){ ++calls; throw std::runtime_error("cannot delete"); };

    SECTION("by reset, which leaves the handle empty")
    {
        {
            unique_handle<int> unique(1, throwing);

            REQUIRE_THROWS_AS(unique.reset(), std::runtime_error);
            REQUIRE(!unique);
            REQUIRE(unique.get() == 0);
        }

        REQUIRE(calls == 1);
    }

    SECTION("by move assignment, which has already taken the other handle")
    {
        int other_calls(0);

        {
            unique_handle<int> unique(1, throwing);
            unique_handle<int> other(2, [&](const int){ ++other_calls; });

            REQUIRE_THROWS_AS(unique = std::move(other), std::runtime_error);
            REQUIRE(unique.get() == 2);
            REQUIRE(unique);
            REQUIRE(!other);
        }

        REQUIRE(calls == 1);
        REQUIRE(other_calls == 1);
    }
}

TEST_CASE("a deleter is given the handle it was made with", "[jfc::unique_handle][jfc::shared_handle]")
{
    std::vector<int> deleted;

    const auto record = [&](const int aHandle){ deleted.push_back(aHandle); };

    {
        const unique_handle<int> unique(3, record);
        const shared_handle<int> shared(4, record);
        const shared_handle<int> converted(unique_handle<int>(5, record));
    }

    REQUIRE(deleted == std::vector<int>{5, 4, 3});
}

TEST_CASE("release gives up the handle without deleting it", "[jfc::unique_handle]")
{
    int calls(0);

    const auto captured = std::make_shared<int>(0);

    SECTION("an owner gives back its handle and becomes empty")
    {
        {
            unique_handle<int> unique(6, [&calls, captured](const int){ ++calls; });

            REQUIRE(unique.release() == 6);
            REQUIRE(!unique);
            REQUIRE(unique.get() == 0);
            REQUIRE(captured.use_count() == 1);
        }

        REQUIRE(calls == 0);
    }

    SECTION("an empty one gives back the null handle")
    {
        unique_handle<int> unique;

        REQUIRE(unique.release() == 0);
        REQUIRE(!unique);
    }

    SECTION("a released one makes an empty shared_handle")
    {
        unique_handle<int> unique(6, [&](const int){ ++calls; });

        static_cast<void>(unique.release());

        const shared_handle<int> shared(std::move(unique));

        REQUIRE(!shared);
        REQUIRE(calls == 0);
    }

    SECTION("a released one can be given another handle")
    {
        {
            unique_handle<int> unique(6, [&](const int){ ++calls; });

            static_cast<void>(unique.release());

            unique.reset(7, [&](const int aHandle){ calls += aHandle; });
        }

        REQUIRE(calls == 7);
    }
}
