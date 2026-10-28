// © Joseph Cameron - MIT License

#include <jfc/shared_handle.h>
#include <jfc/unique_handle.h>
#include <jfc/weak_handle.h>

#include <utility>

int main() {
    int deleted = 0;

    jfc::weak_handle<unsigned> weak;

    {
        jfc::unique_handle<unsigned> unique(3, [&](const unsigned aHandle) { deleted += static_cast<int>(aHandle); });

        const jfc::shared_handle<unsigned> shared(std::move(unique));

        weak = shared;

        if (weak.lock().get() != 3) return 1;
    }

    if (deleted != 3 || weak.lock()) return 1;

    jfc::unique_handle<unsigned> released(4, [&](const unsigned) { ++deleted; });

    return released.release() == 4 && deleted == 3 ? 0 : 1;
}
