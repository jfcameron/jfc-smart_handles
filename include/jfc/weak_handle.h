// © 2019 Joseph Cameron - All Rights Reserved

#ifndef JFC_WEAK_HANDLE_H
#define JFC_WEAK_HANDLE_H

#include <jfc/shared_handle.h>

#include <memory>
#include <optional>

namespace jfc
{
    /// \brief non-owning handle to a resource. The resource may fall out of scope while this is in scope.
    /// to access the resource, a shared_handle must be created via lock method.
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
        std::optional<shared_handle<handle_type>> lock() const
        {
            if (auto pDeleter = m_pDeleter.lock()) 
            {
                auto handle(m_Handle);

                return shared_handle<handle_type>(handle, pDeleter);
            }

            return {};
        }

        bool expired()
        {
            return m_pDeleter.expired();
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
            return weak_handle(std::move(b));
        }

        /// \brief weak handle from shared handle copy semantics
        weak_handle(const shared_handle_type &handle)
        : m_Handle(handle.m_Handle)
        , m_pDeleter(handle.m_pDeleter)
        {}
        /// \brief weak handle from shared handle copy semantics
        //operator= for shared
        weak_handle &operator=(const shared_handle_type &handle) {return weak_handle(handle);}

        ~weak_handle() = default;
    };
}

#endif
