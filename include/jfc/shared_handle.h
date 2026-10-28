// © Joseph Cameron - MIT License

#ifndef JFC_SHARED_HANDLE_H
#define JFC_SHARED_HANDLE_H

#include <jfc/unique_handle.h>

#include <cstddef>
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

        /// \brief alias for deleter functor. Any callable taking the handle will do
        using deleter_type = std::function<void(const handle_type)>;

    private: 
        /// \brief handle to the resource. Value-initialised so an empty instance reports the null handle.
        handle_type m_Handle{};

        /// \brief deletes the handle when the last co-owner lets go of it
        struct control {
            handle_type handle;

            explicit control(const handle_type aHandle) noexcept
            : handle(aHandle)
            {}

            control(const control &) = delete;
            control &operator=(const control &) = delete;

            virtual ~control() = default;
        };

        template<class deleter_param>
        struct control_with final : control {
            deleter_param deleter;

            control_with(const handle_type aHandle, deleter_param &&aDeleter)
            : control(aHandle)
            , deleter(std::move(aDeleter))
            {}

            ~control_with() override { deleter(this->handle); }
        };

        std::shared_ptr<control> m_pControl;

        template<class deleter_param>
        [[nodiscard]] static bool is_empty(const deleter_param &aDeleter) noexcept {
            if constexpr (std::is_assignable_v<deleter_param &, std::nullptr_t>) return aDeleter == nullptr;
            else return false;
        }

        /// \brief tag for the weak->shared constructor. 
        struct from_weak_t { explicit from_weak_t() = default; };

        /// \brief used when promoting a weak to a shared
        shared_handle(from_weak_t, handle_type aHandle, std::shared_ptr<control> aControl) noexcept
        : m_Handle(aHandle)
        , m_pControl(std::move(aControl))
        {}

    public:
        /// \brief an empty handle. 
        shared_handle() noexcept = default;

        /// \brief true when this instance shares ownership of a resource
        [[nodiscard]] explicit operator bool() const noexcept {
            return static_cast<bool>(m_pControl);
        }

        /// \brief give up this instance's share of the resource and become empty
        void reset() noexcept {
            m_Handle = handle_type{};
            m_pControl.reset();
        }

        /// \brief give up this instance's share and take ownership of another resource
        template<class deleter_param>
        requires std::is_invocable_v<deleter_param &, handle_type>
        void reset(handle_type aValue, deleter_param aDeleter) {
            *this = shared_handle(aValue, std::move(aDeleter));
        }

        /// \brief gets a copy of the handle's value
        [[nodiscard]] handle_type get() const noexcept {
            return m_Handle;
        }

        /// \brief returns number of coowners of this handle
        [[nodiscard]] long use_count() const noexcept {
            return m_pControl.use_count();
        }
        
        /// \brief equality operators
        [[nodiscard]] bool operator==(const shared_handle<handle_type> &b) const noexcept {
            return 
                m_Handle   == b.m_Handle && 
                m_pControl == b.m_pControl;
        }

        /// \brief copy semantics
        shared_handle(const shared_handle<handle_type> &b) = default;
        /// \brief copy semantics. 
        shared_handle &operator=(const shared_handle<handle_type> &b) = default;

        /// \brief move semantics. 
        shared_handle(shared_handle<handle_type> &&b) noexcept
        : m_Handle(std::exchange(b.m_Handle, handle_type{}))
        , m_pControl(std::move(b.m_pControl))
        {}
        /// \brief move semantics
        shared_handle &operator=(shared_handle<handle_type> &&b) noexcept {
            if (this != &b) {
                m_Handle = std::exchange(b.m_Handle, handle_type{});
                m_pControl = std::move(b.m_pControl);
            }
            return *this;
        }

        /// \brief move unique_handle to shared_handle semantics. An empty unique_handle gives an empty
        /// shared_handle; if the control block cannot be made, the unique_handle keeps its handle.
        template<class deleter_param>
        shared_handle(unique_handle<handle_type, deleter_param> &&aUnique) {
            if (!aUnique.m_IsOwner) return;

            m_pControl = std::make_shared<control_with<deleter_param>>(aUnique.m_Handle, std::move(aUnique.m_Deleter));
            m_Handle = std::exchange(aUnique.m_Handle, handle_type{});

            aUnique.m_IsOwner = false;
            aUnique.forget_deleter();
        }
        /// \brief move unique_handle to shared_handle semantics
        template<class deleter_param>
        shared_handle &operator=(unique_handle<handle_type, deleter_param> &&aUnique) {
            return *this = shared_handle(std::move(aUnique));
        }

        /// \brief takes a handle and a functor which contains cleanup logic for the handle.
        /// cleanup is delayed to when the final instance of shared_handle which owns the same resource
        /// falls out of scope. If the control block cannot be made, the handle is deleted, as
        /// std::shared_ptr deletes its pointer.
        template<class deleter_param>
        requires std::is_invocable_v<deleter_param &, handle_type>
        shared_handle(handle_type aValue, deleter_param aDeleter)
        : m_Handle(aValue) {
            if (is_empty(aDeleter)) throw std::invalid_argument("jfc::shared_handle: deleter must not be empty");

            try {
                m_pControl = std::make_shared<control_with<deleter_param>>(aValue, std::move(aDeleter));
            }
            catch (...) {
                aDeleter(aValue);

                throw;
            }
        }

        /// \brief the control block invokes the deleter; there is nothing left to do here
        ~shared_handle() noexcept = default;
    };
}

#endif
