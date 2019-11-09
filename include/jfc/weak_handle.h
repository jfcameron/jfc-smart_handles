// © 2019 Joseph Cameron - All Rights Reserved

#ifndef GDK_WEAK_HANDLE_H
#define GDK_WEAK_HANDLE_H

#include <jfc/shared_handle.h>

#include <memory>
#include <optional>

namespace jfc
{
    template<class handle_type_param>
    class weak_handle final
    {
    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;

        /// \brief shared type equivalent
        using shared_handle_type = shared_handle<handle_type>;

    private:
        /// \brief handle to the resource
        handle_type m_Handle;

        /// \brief wp to the deleter
        std::weak_ptr<typename shared_handle<handle_type>::deleter_type> m_pDeleter;

    public:
        /// \brief attempts to create a shared_handle instance, if the handle has not been deleted
        std::optional<shared_handle<handle_type>> lock() //const
        {
            if (auto pDeleter = m_pDeleter.lock()) 
            {
                auto handle(m_Handle);

                return shared_handle<handle_type>(std::move(handle), std::move(*pDeleter));
            }

            return {};
        }
       
        /// \brief equality operators
        bool operator==(const weak_handle<handle_type> &b) const
        {
            return 
                m_Handle   == b.m_Handle && 
                m_pDeleter == b.m_pDeleter;
        }
        
        /// \brief equality operators
        bool operator!=(const weak_handle<handle_type> &b) const
        {
            return 
                m_Handle   != b.m_Handle || 
                m_pDeleter != b.m_pDeleter;
        }

        /// \brief copy semantics
        weak_handle(const weak_handle<handle_type> &b)
        : m_Handle(b.get())
        , m_pDeleter(b.m_pDeleter)
        {}
        /// \brief copy semantics
        weak_handle &operator=(const weak_handle<handle_type> &b) const
        {
            return weak_handle(b);
        }

        /// \brief move semantics
        weak_handle(weak_handle<handle_type> &&b)
        : m_Handle(std::move(b.m_Handle))
        , m_pDeleter(std::move(b.m_pDeleter))
        {}
        /// \brief move semantics
        weak_handle &operator=(weak_handle<handle_type> &&b) const
        {
            return std::move(b);
        }

        //operator= for shared

        /// \brief create a weak handle out of a shared handle
        weak_handle(const shared_handle_type handle)
        : m_Handle(handle.m_Handle)
        , m_pDeleter(handle.m_pDeleter)
        {}

        ~weak_handle() = default;
    };
}

#endif
