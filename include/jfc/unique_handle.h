// © Joseph Cameron - All Rights Reserved

#ifndef JFC_UNIQUE_HANDLE_H
#define JFC_UNIQUE_HANDLE_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace jfc {
    template<typename T> class shared_handle;

    /// \brief move friendly, single owner smart handle. When the owner falls out of scope,
    /// the handle is cleaned up via the user defined deleter 
    template<class handle_type_param>
    class unique_handle final {
        static_assert(
            std::is_trivially_copyable<handle_type_param>::value &&
            std::is_trivially_default_constructible<handle_type_param>::value,
            "handle type must be trivial"
        );

        /// \brief shared must have full access to data in order to move a unique to a shared
        friend shared_handle<handle_type_param>;

    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;
        
        /// \brief alias for deleter functor
        using deleter_type = std::function<void(const handle_type)>;

    private:
        bool m_IsOwner = false;

        /// \brief handle to the resource.
        handle_type m_Handle{};
        
        /// \brief the deletor. Called when a non-moved instance falls out of scope (the owner)
        deleter_type m_Deleter;

        [[nodiscard]] static deleter_type validated(deleter_type aDeleter) {
            if (!aDeleter) throw std::invalid_argument("jfc::unique_handle: deleter must not be empty");

            return aDeleter;
        }

    public:
        /// \brief an empty handle. Owns nothing, deletes nothing.
        unique_handle() noexcept = default;

        /// \brief true when this instance owns a resource. A moved-from instance is false.
        [[nodiscard]] explicit operator bool() const noexcept {
            return m_IsOwner;
        }

        /// \brief release the resource and become empty
        void reset() {
            if (m_IsOwner) m_Deleter(m_Handle);

            m_IsOwner = false;
            m_Handle = handle_type{};
            m_Deleter = deleter_type();
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
        [[nodiscard]] bool operator==(const unique_handle<handle_type> &b) const noexcept {
            return m_Handle == b.m_Handle;
        }

        /// \brief move semantics
        unique_handle(unique_handle<handle_type> &&b) noexcept
        : m_IsOwner(b.m_IsOwner)
        , m_Handle(b.m_Handle)
        , m_Deleter(std::move(b.m_Deleter)) 
        {
            b.m_IsOwner = false;
            b.m_Handle = handle_type{};
        }

        /// \brief move semantics. Not noexcept: it releases the handle this instance owns, which
        /// runs the user's deleter.
        unique_handle &operator=(unique_handle<handle_type> &&b) {
            if (this != &b) {
                if (m_IsOwner) m_Deleter(m_Handle);

                m_IsOwner = b.m_IsOwner;
                m_Handle = b.m_Handle;
                m_Deleter = std::move(b.m_Deleter);

                b.m_IsOwner = false;
                b.m_Handle = handle_type{};
            }

            return *this;
        }

        /// \brief explicitly disallowing copy semantics
        unique_handle(const unique_handle<handle_type> &) = delete;
        /// \brief explicitly disallowing copy semantics
        unique_handle &operator=(const unique_handle<handle_type> &) = delete;

        /// \brief standard constructor
        unique_handle(handle_type aValue, deleter_type aDeleter)
        : m_IsOwner(true)
        , m_Handle(aValue)
        , m_Deleter(validated(std::move(aDeleter)))
        {}

        /// \brief dtor only calls deleter if it is the handle owner.
        ~unique_handle() noexcept {
            if (m_IsOwner) m_Deleter(m_Handle);
        }
    };
}

#endif
