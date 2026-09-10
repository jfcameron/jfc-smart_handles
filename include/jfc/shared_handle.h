// © Joseph Cameron - All Rights Reserved

#ifndef JFC_SHARED_HANDLE_H
#define JFC_SHARED_HANDLE_H

#include <jfc/unique_handle.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace jfc {
    template<typename T> class weak_handle;

    /// \brief provides automatic based mechanism for cleanup of "handle types"
    /// a handle is responsible for some sort of manual state managemnet at end of life. e.g: OpenGL/AL buffer handles, LibClang nodes
    /// Deleter will only be called once: when final the copy of the smart handle goes out of scope.
    /// analogous to stl shared_ptr
    template<class handle_type_param>
    class shared_handle final {
        static_assert(
            std::is_trivially_copyable<handle_type_param>::value &&
            std::is_trivially_default_constructible<handle_type_param>::value,
            "handle type must be trivial"
        );

        /// \brief weaks must have full access to data in order to construct
        friend weak_handle<handle_type_param>;
    public:
        /// \brief alias for handle type
        using handle_type = handle_type_param;

        /// \brief alias for deleter functor
        using deleter_type = std::function<void(const handle_type)>;

    private: 
        /// \brief handle to the resource. Value-initialised so an empty instance reports the null handle.
        handle_type m_Handle{};

        /// \brief shared ownership of the deleter. Its control block owns the cleanup: when the
        /// last co-owner releases, the shared_ptr's own deleter runs the user's deleter
        std::shared_ptr<deleter_type> m_pDeleter;

        /// \brief tag for the weak->shared constructor. 
        struct from_weak_t { explicit from_weak_t() = default; };

        /// \brief used when promoting a weak to a shared
        shared_handle(from_weak_t, handle_type aHandle, std::shared_ptr<deleter_type> aDeleter) noexcept
        : m_Handle(aHandle)
        , m_pDeleter(std::move(aDeleter))
        {}

        /// \brief builds the control block whose destruction invokes aDeleter against aValue.
        [[nodiscard]] static std::shared_ptr<deleter_type> make_control(handle_type aValue, deleter_type aDeleter) {
            if (!aDeleter) throw std::invalid_argument("jfc::shared_handle: deleter must not be empty");

            return std::shared_ptr<deleter_type>(new deleter_type(std::move(aDeleter)),
                [aValue](deleter_type *p) {
                    const std::unique_ptr<deleter_type> owned(p);
                    (*p)(aValue);
                }
            );
        }

    public:
        /// \brief an empty handle. 
        shared_handle() noexcept = default;

        /// \brief true when this instance shares ownership of a resource
        [[nodiscard]] explicit operator bool() const noexcept {
            return static_cast<bool>(m_pDeleter);
        }

        /// \brief give up this instance's share of the resource and become empty
        void reset() noexcept {
            m_Handle = handle_type{};
            m_pDeleter.reset();
        }

        /// \brief give up this instance's share and take ownership of another resource
        void reset(handle_type aValue, deleter_type aDeleter) {
            *this = shared_handle(aValue, std::move(aDeleter));
        }

        /// \brief gets a copy of the handle's value
        [[nodiscard]] handle_type get() const noexcept {
            return m_Handle;
        }

        /// \brief returns number of coowners of this handle
        [[nodiscard]] long use_count() const noexcept {
            return m_pDeleter.use_count();
        }
        
        /// \brief equality operators
        [[nodiscard]] bool operator==(const shared_handle<handle_type> &b) const noexcept {
            return 
                m_Handle   == b.m_Handle && 
                m_pDeleter == b.m_pDeleter;
        }

        /// \brief copy semantics
        shared_handle(const shared_handle<handle_type> &b) = default;
        /// \brief copy semantics. 
        shared_handle &operator=(const shared_handle<handle_type> &b) = default;

        /// \brief move semantics. 
        shared_handle(shared_handle<handle_type> &&b) noexcept
        : m_Handle(b.m_Handle)
        , m_pDeleter(std::move(b.m_pDeleter)) 
        {
            b.m_Handle = handle_type{};
        }
        /// \brief move semantics
        shared_handle &operator=(shared_handle<handle_type> &&b) noexcept {
            if (this != &b) {
                m_Handle = b.m_Handle;
                m_pDeleter = std::move(b.m_pDeleter);
                b.m_Handle = handle_type{};
            }
            return *this;
        }

        /// \brief move unique_handle to shared_handle semantics
        shared_handle(unique_handle<handle_type> &&aUnique)
        : m_Handle(aUnique.m_Handle)
        , m_pDeleter(make_control(aUnique.m_Handle, std::move(aUnique.m_Deleter)))
        {
            aUnique.m_IsOwner = false;
        }
        /// \brief move unique_handle to shared_handle semantics
        shared_handle &operator=(unique_handle<handle_type> &&aUnique) {
            return *this = shared_handle(std::move(aUnique));
        }

        /// \brief takes a handle and a functor which contains cleanup logic for the handle.
        /// cleanup is delayed to when the final instance of shared_handle which owns the same resource
        /// falls out of scope.
        shared_handle(handle_type aValue, deleter_type aDeleter)
        : m_Handle(aValue)
        , m_pDeleter(make_control(aValue, std::move(aDeleter)))
        {}

        /// \brief the control block invokes the deleter; there is nothing left to do here
        ~shared_handle() noexcept = default;
    };
}

#endif
