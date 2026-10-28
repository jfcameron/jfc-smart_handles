// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>
#include <jfc/weak_handle.h>

#include <utility>
#include <vector>

using namespace jfc;

namespace
{
    using GLuint = unsigned int;

    struct FT_FaceRec final { int glyphs; };

    using FT_Face = FT_FaceRec *;

    struct context final
    {
        void *device;
        unsigned id;

        bool operator==(const context &) const = default;
    };

    template<typename Handle>
    void through_every_kind(const Handle aFirst, const Handle aSecond)
    {
        std::vector<Handle> deleted;

        const auto record = [&deleted](const Handle aHandle) { deleted.push_back(aHandle); };

        {
            unique_handle<Handle> unique(aFirst, record);

            REQUIRE(unique.get() == aFirst);

            shared_handle<Handle> shared(std::move(unique));

            REQUIRE(unique.get() == Handle{});

            const weak_handle<Handle> weak(shared);

            REQUIRE(weak.lock().get() == aFirst);

            shared.reset(aSecond, record);

            REQUIRE(weak.expired());
            REQUIRE(shared.get() == aSecond);
            REQUIRE(deleted == std::vector<Handle>{aFirst});
        }

        REQUIRE(deleted == std::vector<Handle>{aFirst, aSecond});
    }
}

TEST_CASE("handles of the types libraries use", "[jfc::unique_handle][jfc::shared_handle][jfc::weak_handle]")
{
    SECTION("an unsigned name, as OpenGL and OpenAL give")
    {
        through_every_kind<GLuint>(1u, 0xFFFFFFFFu);
    }

    SECTION("a pointer, as FreeType gives")
    {
        FT_FaceRec first{1}, second{2};

        through_every_kind<FT_Face>(&first, &second);
    }

    SECTION("a struct of values")
    {
        int device(0);

        through_every_kind<context>(context{&device, 1}, context{nullptr, 2});
    }
}
