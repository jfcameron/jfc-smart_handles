// © 2019 Joseph Cameron - All Rights Reserved

#ifndef JFC_UNIQUE_HANDLE_H
#define JFC_UNIQUE_HANDLE_H

#include <functional>
#include <memory>

namespace jfc
{
    template<typename T> class shared_handle;

    /// \brief move friendly, single owner smart handle. When the owner falls out of scope,
    /// the handle is cleaned up via the deleter call in the dtor.
    template<class handle_type_param>
    class unique_handle final
    {
        static_assert(std::is_trivial<handle_type_param>::value, "handle type must be trivial");

        /// \brief shared must have full access to data in order to move a unique to a shared
        friend shared_handle<handle_type_param>;

    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;
        
        /// \brief alias for deleter functor
        using deleter_type = std::function<void(const handle_type)>;

    private:
        /// \brief a moved instance no longer owns the resource
        bool m_IsOwner = true;

        /// \brief handle to the resource
        handle_type m_Handle;
        
        /// \brief the deletor. Called when a non-moved instance falls out of scope (the owner)
        deleter_type m_Deleter;

    public:
        /// \brief get a copy of the handle
        [[nodiscard]] handle_type get() const noexcept
        {
            return m_Handle;
        }

        /// \brief equality semantics
        [[nodiscard]] bool operator==(const unique_handle<handle_type> &b) const noexcept
        {
            return m_Handle == b.m_Handle;
        }
        /// \brief equality semantics
        [[nodiscard]] bool operator!=(const unique_handle<handle_type> &b) const noexcept {return !(*this = b);}

        /// \brief move semantics
        unique_handle(unique_handle<handle_type> &&b)
        : m_Handle(std::move(b.m_Handle))
        , m_Deleter(std::move(b.m_Deleter))
        , m_IsOwner(b.m_IsOwner)
        {
            b.m_IsOwner = false;
        }
        /// \brief move semantics
        unique_handle &operator=(unique_handle<handle_type> &&b) const {return std::move(b);}

        /// \brief explicitly disallowing copy semantics
        unique_handle(const unique_handle<handle_type> &) = delete;
        /// \brief explicitly disallowing copy semantics
        unique_handle &operator=(const unique_handle<handle_type> &) = delete;

        /// \brief standard constructor
        unique_handle(handle_type aValue, deleter_type aDeleter)
        : m_Handle(aValue)
        , m_Deleter(aDeleter)
        {}

        /// \brief dtor only calls deleter if it is the handle owner
        ~unique_handle() noexcept
        {
            if (m_IsOwner) m_Deleter(m_Handle);
        }
    };
}

#endif
