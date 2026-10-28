// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>
#include <jfc/weak_handle.h>

#include <atomic>
#include <thread>
#include <utility>
#include <vector>

using namespace jfc;

TEST_CASE("shares copied and dropped on many threads delete the handle once", "[jfc::shared_handle][jfc::weak_handle][threads]")
{
    std::atomic<int> calls(0);
    std::atomic<bool> misread(false);

    {
        shared_handle<int> shared(7, [&calls](const int){ ++calls; });
        const weak_handle<int> weak(shared);

        std::vector<std::thread> threads;

        for (int each = 0; each < 8; ++each)
        {
            threads.emplace_back([shared, weak, &misread]()
            {
                for (int i = 0; i < 2000; ++i)
                {
                    auto copy = shared;
                    const weak_handle<int> observer(copy);

                    if (const auto locked = weak.lock(); locked && locked.get() != 7) misread = true;
                    if (observer.lock().get() != 7) misread = true;

                    copy.reset();
                }
            });
        }

        shared.reset();

        for (auto &each : threads) each.join();

        REQUIRE(calls == 1);
        REQUIRE(weak.expired());
        REQUIRE(!misread);
    }

    REQUIRE(calls == 1);
}

TEST_CASE("a unique_handle moved to another thread is deleted there, once", "[jfc::unique_handle][threads]")
{
    std::atomic<int> calls(0);
    std::thread::id deletedOn;

    unique_handle<int> unique(1, [&](const int){ ++calls; deletedOn = std::this_thread::get_id(); });

    std::thread other([moved = std::move(unique)]() mutable { moved.reset(); });

    const auto otherId = other.get_id();

    other.join();

    REQUIRE(!unique);
    REQUIRE(calls == 1);
    REQUIRE(deletedOn == otherId);
}
