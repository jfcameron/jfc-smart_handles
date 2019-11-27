// © 2019 Joseph Cameron - All Rights Reserved

#ifndef JFC_SHARED_HANDLE_H
#define JFC_SHARED_HANDLE_H

#include <jfc/unique_handle.h>

#include <functional>
#include <memory>

namespace jfc
{
    template<typename T> class weak_handle;

    /// \brief provides automatic based mechanism for cleanup of "handle types"
    /// a handle is responsible for some sort of manual state managemnet at end of life. e.g: OpenGL/AL buffer handles, LibClang nodes
    /// Deleter will only be called once: when final the copy of the smart handle goes out of scope.
    /// Implementation based on final_action found in the Cpp General Guidelines, extended to strongly associate the handle with the deletor (both are members) and to support copy operations in addition to moves
    /// analogous to stl shared_ptr
    template<class handle_type_param>
    class shared_handle final
    {
        static_assert(std::is_trivial<handle_type_param>::value, "handle type must be trivial");

        /// \brief weaks must have full access to data in order to construct
        friend weak_handle<handle_type_param>;
    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;

        /// \brief alias for deleter functor
        using deleter_type = std::function<void(const handle_type)>;

    private: 
        /// \brief handle to the resource
        handle_type m_Handle;

        /// \brief a pointer to the deletor shared across moved/copied instances. Called when the final co-owning instance of the handle falls out of scope, cleaning up the resource.
        std::shared_ptr<deleter_type> m_pDeleter;

        /// \brief used when promoting a weak to a shared
        shared_handle(decltype(m_Handle) aHandle, decltype(m_pDeleter) aDeleter)
        : m_Handle(aHandle)
        , m_pDeleter(aDeleter)
        {}

    public:
        /// \brief gets a copy of the handle's value
        [[nodiscard]] handle_type get() const noexcept
        {
            return m_Handle;
        }

        /// \brief returns number of coowners of this handle
        [[nodiscard]] long use_count() const noexcept
        {
            return m_pDeleter.use_count();
        }
        
        /// \brief equality operators
        [[nodiscard]] bool operator==(const shared_handle<handle_type> &b) const noexcept
        {
            return 
                m_Handle   == b.m_Handle && 
                m_pDeleter == b.m_pDeleter;
        }
        
        /// \brief equality operators
        [[nodiscard]] bool operator!=(const shared_handle<handle_type> &b) const noexcept
        {
			return !(*this == b);
        }

        /// \brief copy semantics
        shared_handle(const shared_handle<handle_type> &b)
        : m_Handle(b.get())
        , m_pDeleter(b.m_pDeleter)
        {}
        /// \brief copy semantics
        shared_handle &operator=(const shared_handle<handle_type> &b) const
        {
            return shared_handle(b);
        }

        /// \brief move semantics
        shared_handle(shared_handle<handle_type> &&b)
        : m_Handle(std::move(b.m_Handle))
        , m_pDeleter(std::move(b.m_pDeleter))
        {}
        /// \brief move semantics
        shared_handle &operator=(shared_handle<handle_type> &&b) const
        {
            return std::move(b);
        }

        /// \brief move unique_handle to shared_handle semantics
        shared_handle(unique_handle<handle_type> &&unique)
        : m_Handle(std::move(unique.m_Handle))
        , m_pDeleter(std::make_shared<deleter_type>(unique.m_Deleter))
        {
            unique.m_IsOwner = false;
        }
        /// \brief move unique_handle to shared_handle semantics
        shared_handle &operator=(unique_handle<handle_type> &&unique) const
        {
            return std::move(unique);    
        }

        /// \brief takes a handle and a functor which contains cleanup logic for the handle.
        /// cleanup is delayed to when the final instance of shared_handle which owns the same resource
        /// falls out of scope.
        shared_handle(handle_type &&aValue, deleter_type &&aDeleter)
        : m_Handle(aValue)
        , m_pDeleter(std::make_shared<shared_handle<handle_type>::deleter_type>(aDeleter))
        {}

        /// \brief if this is the final owner, invoke the deleter (so long as it exists)
        /// <= used since non-zero values are approximate in a concurrent context
        ~shared_handle() noexcept
        {
            if (m_pDeleter.use_count() <= 1 && m_pDeleter) (*m_pDeleter)(m_Handle);
        }
    };
}

#endif
