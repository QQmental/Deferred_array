#include <initializer_list>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

template <class element_t, std::size_t dimension>
class Deferred_array
{
public:
    
    std::size_t data_len() const 
    {
        std::size_t len = 1;
        for(auto &&bound : m_dimension_boundary)
            len *= bound;
        return len;
    }

    Deferred_array() : m_data(nullptr){}

    Deferred_array(const std::initializer_list<std::size_t> &list) 
    {
        assert(list.size() == dimension);
        
        std::copy(std::begin(list), std::end(list), std::begin(m_dimension_boundary));

        m_data = new element_t[data_len()];
    }

    Deferred_array(const Deferred_array<element_t, dimension> &src)
    {
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));

        auto new_alloc = new char[sizeof(element_t) * data_len()];
        m_data = new(new_alloc)element_t;

        for(std::size_t i = 0 ; i < data_len() ; i++)
        {
            auto dst_ptr = new (m_data)element_t;
            auto src_ptr = new (src.m_data)element_t;
            dst_ptr[i] = src_ptr[i];
        }

        *this = src;
    }

    Deferred_array(Deferred_array<element_t, dimension> &&src)
    {
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
        
        m_data = src.m_data;

        src.m_data = nullptr;
    }
    ~Deferred_array()
    {
        delete[] m_data;
    }
    Deferred_array& operator=(const Deferred_array<element_t, dimension> &src)
    {
        if (this == &src)
            return *this;

        this->~Deferred_array();
        
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
    
        auto new_alloc = new char[sizeof(element_t) * data_len()];
        m_data = new(new_alloc)element_t;

        for(std::size_t i = 0 ; i < data_len() ; i++)
        {
            auto dst_ptr = new (m_data)element_t;
            auto src_ptr = new (src.m_data)element_t;
            dst_ptr[i] = src_ptr[i];
        }
        return *this;
    }

    Deferred_array& operator=(Deferred_array<element_t, dimension> &&src)
    {
        if (this == &src)
            return *this;

        this->~Deferred_array();
        
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
    
        m_data = src.m_data;
        src.m_data = nullptr;

        return *this;
    }

    const element_t* data() const noexcept {return m_data;}
    const element_t* cdata() noexcept {return m_data;}
    element_t* data() noexcept {return m_data;}
    
    auto operator[](int idx) const noexcept 
    {
        auto ret = Deferred_array<element_t, dimension>::bracket_impl<dimension>{m_data, m_dimension_boundary}[idx];
        return ret;
    }

private:
    template<std::size_t cnt>
    struct bracket_impl
    {
        element_t *m_ptr = nullptr;
        const std::size_t (&m_boundary_arr)[dimension];
        std::size_t num = 0;

        bracket_impl(element_t *ptr, const std::size_t (&boundary_arr)[dimension]) : m_ptr(ptr), m_boundary_arr(boundary_arr){};

        bool is_out_of_rangge(std::size_t idx) const
        {
            auto x = m_boundary_arr[dimension - cnt];
            return x <= idx;
        }

        auto operator[](std::size_t idx) const noexcept
        {
            assert(is_out_of_rangge(idx) == false);

            std::size_t dim_size = 1;
            for(std::size_t i = dimension - cnt + 1 ; i < dimension ; i++)
                dim_size *= m_boundary_arr[i];

            Deferred_array<element_t, dimension>::bracket_impl<cnt - 1> ret{m_ptr, m_boundary_arr};
            ret.num = this->num + idx*dim_size;

            if constexpr (cnt > 1)
                return ret;
            else
                return std::ref(ret.m_ptr[ret.num]);
        }
    };



    element_t *m_data;
    std::size_t m_dimension_boundary[dimension] = {0};
};