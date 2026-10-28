// © Joseph Cameron - MIT License

#ifndef JFC_UNIQUE_HANDLE_H
#define JFC_UNIQUE_HANDLE_H

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if defined(__has_cpp_attribute) && __has_cpp_attribute(msvc::no_unique_address)
#define JFC_UNIQUE_HANDLE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#define JFC_UNIQUE_HANDLE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define JFC_UNIQUE_HANDLE_NO_UNIQUE_ADDRESS
#endif

namespace jfc {
    template<typename T> class shared_handle;

    /// \brief move friendly, single owner smart handle. When the owner falls out of scope,
    /// the handle is cleaned up via the user defined deleter
    template<class handle_type_param, class deleter_type_param = std::function<void(const handle_type_param)>>
    class unique_handle final {
        static_assert(
            std::is_trivially_copyable<handle_type_param>::value &&
            std::is_trivially_default_constructible<handle_type_param>::value,
            "handle type must be trivial"
        );

        static_assert(std::is_invocable_v<deleter_type_param &, handle_type_param>,
            "deleter must be callable with the handle");

        /// \brief shared must have full access to data in order to move a unique to a shared
        friend shared_handle<handle_type_param>;

    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;

        /// \brief alias for deleter functor
        using deleter_type = deleter_type_param;

    private:
        // a std::function or a function pointer can be empty
        static constexpr bool NULLABLE_DELETER = std::is_assignable_v<deleter_type &, std::nullptr_t>;

        bool m_IsOwner = false;

        /// \brief handle to the resource.
        handle_type m_Handle{};

        /// \brief the deletor. Called when a non-moved instance falls out of scope (the owner)
        JFC_UNIQUE_HANDLE_NO_UNIQUE_ADDRESS deleter_type m_Deleter{};

        [[nodiscard]] static deleter_type validated(deleter_type aDeleter) {
            if constexpr (NULLABLE_DELETER) {
                if (aDeleter == nullptr) throw std::invalid_argument("jfc::unique_handle: deleter must not be empty");
            }

            return aDeleter;
        }

        void forget_deleter() noexcept {
            if constexpr (NULLABLE_DELETER) m_Deleter = nullptr;
        }

    public:
        /// \brief an empty handle. Owns nothing, deletes nothing.
        unique_handle() = default;

        /// \brief true when this instance owns a resource. A moved-from instance is false.
        [[nodiscard]] explicit operator bool() const noexcept {
            return m_IsOwner;
        }

        /// \brief release the resource and become empty
        void reset() {
            const auto wasOwner = std::exchange(m_IsOwner, false);
            const auto handle = std::exchange(m_Handle, handle_type{});

            if (wasOwner) m_Deleter(handle);

            forget_deleter();
        }

        /// \brief give up the handle without deleting it, and become empty
        [[nodiscard]] handle_type release() noexcept {
            m_IsOwner = false;

            forget_deleter();

            return std::exchange(m_Handle, handle_type{});
        }

        /// \brief release the resource and take ownership of another
        void reset(handle_type aValue, deleter_type aDeleter) {
            *this = unique_handle(aValue, std::move(aDeleter));
        }

        /// \brief get a copy of the handle
        [[nodiscard]] handle_type get() const noexcept {
            return m_Handle;
        }

        /// \brief equality semantics
        [[nodiscard]] bool operator==(const unique_handle &b) const noexcept {
            return m_Handle == b.m_Handle;
        }

        /// \brief move semantics
        unique_handle(unique_handle &&b) noexcept(std::is_nothrow_move_constructible_v<deleter_type>)
        : m_IsOwner(std::exchange(b.m_IsOwner, false))
        , m_Handle(std::exchange(b.m_Handle, handle_type{}))
        , m_Deleter(std::move(b.m_Deleter))
        {
            b.forget_deleter();
        }

        /// \brief move semantics. Not noexcept: it releases the handle this instance owns, which
        /// runs the user's deleter.
        unique_handle &operator=(unique_handle &&b) {
            if (this != &b) {
                const auto wasOwner = std::exchange(m_IsOwner, std::exchange(b.m_IsOwner, false));
                const auto oldHandle = std::exchange(m_Handle, std::exchange(b.m_Handle, handle_type{}));
                auto oldDeleter = std::move(m_Deleter);

                m_Deleter = std::move(b.m_Deleter);

                b.forget_deleter();

                if (wasOwner) oldDeleter(oldHandle);
            }

            return *this;
        }

        /// \brief explicitly disallowing copy semantics
        unique_handle(const unique_handle &) = delete;
        /// \brief explicitly disallowing copy semantics
        unique_handle &operator=(const unique_handle &) = delete;

        /// \brief standard constructor
        unique_handle(handle_type aValue, deleter_type aDeleter)
        : m_IsOwner(true)
        , m_Handle(aValue)
        , m_Deleter(validated(std::move(aDeleter)))
        {}

        /// \brief takes a handle, to be deleted by a deleter that has no state
        explicit unique_handle(handle_type aValue)
        requires (std::is_empty_v<deleter_type> && std::is_default_constructible_v<deleter_type>)
        : m_IsOwner(true)
        , m_Handle(aValue)
        {}

        /// \brief dtor only calls deleter if it is the handle owner.
        ~unique_handle() noexcept {
            if (m_IsOwner) m_Deleter(m_Handle);
        }
    };
}

#undef JFC_UNIQUE_HANDLE_NO_UNIQUE_ADDRESS

#endif
