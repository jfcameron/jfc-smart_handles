// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>
#include <jfc/weak_handle.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace jfc;

namespace
{
    std::vector<unsigned> g_Deleted;

    struct delete_buffer final
    {
        void operator()(const unsigned aHandle) const { g_Deleted.push_back(aHandle); }
    };

    void delete_by_function(const unsigned aHandle) { g_Deleted.push_back(aHandle); }

    using deleted = std::vector<unsigned>;
}

TEST_CASE("a deleter type of its own is held as it is", "[jfc::unique_handle]")
{
    g_Deleted.clear();

    SECTION("and one with no state takes no space")
    {
        REQUIRE(sizeof(unique_handle<unsigned, delete_buffer>) == 2 * sizeof(unsigned));
    }

    SECTION("and one with no state needs only the handle")
    {
        {
            const unique_handle<unsigned, delete_buffer> unique(3);

            REQUIRE(unique);
            REQUIRE(unique.get() == 3);
        }

        REQUIRE(g_Deleted == deleted{3});
    }

    SECTION("moved, reset and released as the default deleter is")
    {
        {
            unique_handle<unsigned, delete_buffer> first(1), second(2);

            first = std::move(second);

            REQUIRE(g_Deleted == deleted{1});
            REQUIRE(first.get() == 2);
            REQUIRE(!second);

            first.reset();

            REQUIRE(g_Deleted == deleted{1, 2});

            first.reset(4, delete_buffer{});

            REQUIRE(first.release() == 4);
        }

        REQUIRE(g_Deleted == deleted{1, 2});
    }

    SECTION("and made shared")
    {
        {
            const shared_handle<unsigned> shared(unique_handle<unsigned, delete_buffer>(5));
            const weak_handle<unsigned> weak(shared);

            REQUIRE(weak.lock().get() == 5);
            REQUIRE(g_Deleted.empty());
        }

        REQUIRE(g_Deleted == deleted{5});
    }
}

TEST_CASE("a function pointer can be the deleter, but not a null one", "[jfc::unique_handle][jfc::shared_handle]")
{
    g_Deleted.clear();

    using pointer = void (*)(unsigned);

    {
        const unique_handle<unsigned, pointer> unique(6, &delete_by_function);
        const shared_handle<unsigned> shared(7, &delete_by_function);
    }

    REQUIRE(g_Deleted == deleted{7, 6});

    REQUIRE_THROWS_AS((unique_handle<unsigned, pointer>(1, nullptr)), std::invalid_argument);
    REQUIRE_THROWS_AS((shared_handle<unsigned>(1, pointer())), std::invalid_argument);
}

TEST_CASE("a deleter that can only be moved, which a std::function cannot hold", "[jfc::unique_handle][jfc::shared_handle]")
{
    std::vector<int> handles;

    auto deleter = [&handles, pOffset = std::make_unique<int>(8)](const int aHandle) { handles.push_back(aHandle + *pOffset); };

    using deleter_type = decltype(deleter);

    SECTION("held by a unique_handle, moved, and made shared")
    {
        {
            unique_handle<int, deleter_type> unique(1, std::move(deleter));
            unique_handle<int, deleter_type> moved(std::move(unique));

            const shared_handle<int> shared(std::move(moved));

            REQUIRE(!unique);
            REQUIRE(!moved);
            REQUIRE(handles.empty());
        }

        REQUIRE(handles == std::vector<int>{9});
    }

    SECTION("held by a shared_handle")
    {
        {
            const shared_handle<int> shared(2, std::move(deleter));
            const auto copy = shared;
        }

        REQUIRE(handles == std::vector<int>{10});
    }
}
