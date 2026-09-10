// © 2019 Joseph Cameron - All Rights Reserved

#ifndef JFC_WEAK_HANDLE_H
#define JFC_WEAK_HANDLE_H

#include <jfc/shared_handle.h>

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace jfc {
    /// \brief non-owning handle to a resource. The resource may fall out of scope while this is in scope.
    /// to access the resource, a shared_handle must be created via lock method.
    template<class handle_type_param>
    class weak_handle final {
        static_assert(
            std::is_trivially_copyable<handle_type_param>::value &&
            std::is_trivially_default_constructible<handle_type_param>::value,
            "handle type must be trivial"
        );

    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;

        /// \brief shared type equivalent
        using shared_handle_type = shared_handle<handle_type>;

    private:
        /// \brief handle to the resource.
        handle_type m_Handle{};

        /// \brief wp to the deleter
        std::weak_ptr<typename shared_handle<handle_type>::deleter_type> m_pDeleter;

    public:
        /// \brief an empty handle
        weak_handle() noexcept = default;

        /// \brief stop observing and become empty
        void reset() noexcept {
            m_Handle = handle_type{};
            m_pDeleter.reset();
        }

        /// \brief attempts to create a shared_handle instance 
        [[nodiscard]] std::optional<shared_handle<handle_type>> lock() const noexcept {
            if (auto pDeleter = m_pDeleter.lock()) {
                return shared_handle<handle_type>(
                    typename shared_handle<handle_type>::from_weak_t{}, m_Handle, std::move(pDeleter));
            }

            return {};
        }
    
        /// \brief checks whether or not the observed shared_handle has fallen out of scope
        [[nodiscard]] bool expired() const noexcept {
            return m_pDeleter.expired();
        }

        /// \brief copy semantics
        weak_handle(const weak_handle<handle_type> &b) = default;
        /// \brief copy semantics. 
        weak_handle &operator=(const weak_handle<handle_type> &b) = default;

        /// \brief move semantics. 
        weak_handle(weak_handle<handle_type> &&b) noexcept
        : m_Handle(b.m_Handle)
        , m_pDeleter(std::move(b.m_pDeleter))
        {
            b.m_Handle = handle_type{};
        }
        /// \brief move semantics
        weak_handle &operator=(weak_handle<handle_type> &&b) noexcept {
            if (this != &b) {
                m_Handle = b.m_Handle;
                m_pDeleter = std::move(b.m_pDeleter);

                b.m_Handle = handle_type{};
            }

            return *this;
        }

        /// \brief weak handle from shared handle copy semantics
        weak_handle(const shared_handle_type &handle)
        : m_Handle(handle.m_Handle)
        , m_pDeleter(handle.m_pDeleter)
        {}
        /// \brief weak handle from shared handle copy semantics
        weak_handle &operator=(const shared_handle_type &handle) {
            m_Handle = handle.m_Handle;
            m_pDeleter = handle.m_pDeleter;

            return *this;
        }

        ~weak_handle() noexcept = default;
    };
}

#endif
