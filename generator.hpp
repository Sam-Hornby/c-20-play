#include <coroutine>
#include <optional>
#include <assert.h>



template <class T, class promise_type_def>
class GeneratorImpl {
public:
    using promise_type = promise_type_def;
    using Handle = std::coroutine_handle<promise_type>;
 
    explicit GeneratorImpl(const Handle coroutine) : 
        m_coroutine{coroutine}
    {}
 
    GeneratorImpl() = default;
    ~GeneratorImpl() { 
        if (m_coroutine) {
            m_coroutine.destroy(); 
        }
    }
 
    GeneratorImpl(const GeneratorImpl&) = delete;
    GeneratorImpl& operator=(const GeneratorImpl&) = delete;
 
    GeneratorImpl(GeneratorImpl&& other) noexcept : 
        m_coroutine{other.m_coroutine}
    { 
        other.m_coroutine = {}; 
    }
    GeneratorImpl& operator=(GeneratorImpl&& other) noexcept {
        if (this != &other) {
            if (m_coroutine) {
                m_coroutine.destroy();
            }
            m_coroutine = other.m_coroutine;
            other.m_coroutine = {};
        }
        return *this;
    }
 
    // Range-based for loop support.
    class Iter {
    public:
        void operator++() { 
            m_coroutine.resume(); 
        }
        const T& operator*() const { 
            return *m_coroutine.promise().current_value; 
        }        
        bool operator==(const Iter & other) const {
            // only valid to compare against end iterator
            assert(!other.m_coroutine || other.m_coroutine.done());
            return !m_coroutine || m_coroutine.done();
        }
 
        explicit Iter(const Handle coroutine) : 
            m_coroutine{coroutine}
        {}
 
        Handle m_coroutine;
    };
 
    Iter begin() {
        if (m_coroutine) {
            m_coroutine.resume();
        } 
        return Iter{m_coroutine};
    }
    Iter end() { 
        return Iter{Handle()}; 
    }
 
private:
    Handle m_coroutine;
};

template <class T>
struct promise_type {
    GeneratorImpl<T, promise_type> get_return_object() {
        return GeneratorImpl<T, promise_type>{
            GeneratorImpl<T, promise_type>::Handle::from_promise(*this)};
    }
    static std::suspend_always initial_suspend() noexcept {
        return {}; 
    }
    static std::suspend_always final_suspend() noexcept { 
        return {}; 
    }
    std::suspend_always yield_value(T value) noexcept {
        current_value = std::move(value);
        return {};
    }

    // Disallow co_await in generator coroutines.
    void await_transform() = delete;
    [[noreturn]] static void unhandled_exception() {
        throw;
    }

    std::optional<T> current_value;
};

template <class T>
struct promise_type_with_return : public promise_type<T> {
    void return_value(T value) noexcept {
        this->current_value = std::move(value);
    }
    GeneratorImpl<T, promise_type_with_return> get_return_object() {
        return GeneratorImpl<T, promise_type_with_return>{
            GeneratorImpl<T, promise_type_with_return>::Handle::from_promise(*this)};
    }
};

template <class T>
using Generator = GeneratorImpl<T, promise_type<T>>;

template <class T>
using ReturnGenerator = GeneratorImpl<T, promise_type_with_return<T>>;
