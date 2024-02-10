#include <initializer_list>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

template <class element_t, std::size_t dimension>
class Deferred_array
{
public:
    
    inline static constexpr std::size_t dimension_length = dimension; 
    std::size_t data_len() const 
    {
        return m_data_len;
    }

    Deferred_array() : m_data(nullptr), m_data_len(0){}

    Deferred_array(const std::initializer_list<std::size_t> &list) 
    {
        assert(list.size() == dimension);
        
        std::copy(std::begin(list), std::end(list), std::begin(m_dimension_boundary));

        m_data_len = 1;
        for(auto &&bound : m_dimension_boundary)
            m_data_len *= bound;
        
        m_data = new element_t[data_len()];
    }

    Deferred_array(const Deferred_array<element_t, dimension> &src)
    {
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
        m_data_len = src.data_len();

        auto new_alloc = new char[sizeof(element_t) * data_len()];
        m_data = new(new_alloc)element_t;

        for(std::size_t i = 0 ; i < data_len() ; i++)
        {
            auto dst_ptr = new (m_data)element_t;
            auto src_ptr = new (src.m_data)element_t;
            dst_ptr[i] = src_ptr[i];
        }
    }

    Deferred_array(Deferred_array<element_t, dimension> &&src)
    {
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));

        m_data = src.m_data;
        m_data_len = src.m_data_len;

        src.m_data = nullptr;
        src.m_data_len = 0;
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
        m_data_len = src.m_data_len;

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
        m_data_len = src.m_data_len;

        src.m_data = nullptr;
        src.m_data_len = 0 ;

        return *this;
    }

    const element_t* data() const noexcept {return m_data;}
    const element_t* cdata() noexcept {return m_data;}
    element_t* data() noexcept {return m_data;}
    
    auto operator[](int idx) const noexcept 
    {
        auto ptr_stride_length = data_len()/m_dimension_boundary[0];
        auto ret = Deferred_array<element_t, dimension>::Ptr_impl<dimension>{m_data, m_dimension_boundary, ptr_stride_length}[idx];
        return ret;
    }

private:
    template<std::size_t cnt>
    class Ptr_impl
    {
    private:
        element_t *m_ptr = nullptr;
        const std::size_t (&m_boundary_arr)[dimension];
        std::size_t m_stride_length;

    public:
        Ptr_impl(element_t *ptr, const std::size_t (&boundary_arr)[dimension], std::size_t stride_length) 
        : m_ptr(ptr), 
          m_boundary_arr(boundary_arr),
          m_stride_length(stride_length){};

        const element_t& operator*() const noexcept {return *m_ptr;}
        element_t& operator*() noexcept {return *m_ptr;}
        const element_t* operator->() const noexcept {return m_ptr;}
        element_t* operator->() noexcept {return m_ptr;}

        bool is_out_of_rangge(std::size_t idx) const
        {
            auto x = m_boundary_arr[dimension - cnt];
            return x <= idx;
        }

        decltype(auto) operator[](std::size_t idx) const noexcept
        {
            //assert(is_out_of_rangge(idx) == false);
            
            if constexpr (cnt > 1)
            {
                auto next_stride_length = m_stride_length/m_boundary_arr[dimension-cnt+1];

                Deferred_array<element_t, dimension>::Ptr_impl<cnt - 1> ret{m_ptr + idx*m_stride_length, m_boundary_arr, next_stride_length};
            
                return ret;
            }
            else
                return m_ptr[idx];
        }
        friend Deferred_array<element_t, dimension>::Ptr_impl<cnt+1>;
    };
    element_t *m_data;
    std::size_t m_dimension_boundary[dimension] = {0};
    std::size_t m_data_len;
};
