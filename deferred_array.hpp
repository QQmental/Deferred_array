#include <initializer_list>
#include <new>
#include <memory>
#include <array>
template <class element_t, std::size_t dimension>
class Deferred_array
{
public:
    
    inline static constexpr std::size_t dimension_length = dimension; 
    std::size_t data_len() const 
    {
        return m_data_len;
    }

    Deferred_array() : m_data(nullptr), m_dimension_boundary(), m_data_len(0){}

    Deferred_array(const std::initializer_list<std::size_t> &list) 
    {
               
       std::copy(std::begin(list), std::end(list), std::begin(m_dimension_boundary));

        m_data_len = 1;
        for(auto &&bound : m_dimension_boundary)
        {
            m_data_len *= bound;
        }
    
        m_data = reinterpret_cast<element_t*>(::operator new[](sizeof(element_t) * data_len()));
        
        std::uninitialized_default_construct_n(m_data, data_len());
        init_stride_size();
    }

    Deferred_array(const Deferred_array<element_t, dimension> &src)
                  :  m_data_len(src.data_len())
    {
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));

        std::copy(std::begin(src.m_stride_size), std::end(src.m_stride_size), std::begin(m_stride_size));

        m_data = reinterpret_cast<element_t*>(::operator new[](sizeof(element_t) * data_len()));
        
        std::uninitialized_copy_n(src.data(), src.data_len(), data());
    }

    Deferred_array(Deferred_array<element_t, dimension> &&src) noexcept
                  :  m_dimension_boundary(src.m_dimension_boundary),
                     m_data_len(src.data_len())
    {

        m_data = src.m_data;

        src.m_data = nullptr;
        src.m_data_len = 0;
        src.m_dimension_boundary = {};
        src.m_stride_size = {};
    }
    ~Deferred_array()
    {
        for(std::size_t i = 0 ; i < data_len() ; i++)
            m_data[i].~element_t();
        ::operator delete[](m_data) ;
    }
    Deferred_array& operator=(const Deferred_array<element_t, dimension> &src)
    {
        if (this == &src)
            return *this;

        this->~Deferred_array();
        
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
        m_data_len = src.m_data_len;
        std::copy(std::begin(src.m_stride_size), std::end(src.m_stride_size), std::begin(m_stride_size));

        m_data = reinterpret_cast<element_t*>(::operator new[](sizeof(element_t) * data_len()));
        
        std::uninitialized_copy_n(src.data(), src.data_len(), data());
    
        return *this;
    }

    Deferred_array& operator=(Deferred_array<element_t, dimension> &&src) noexcept 
    {
        if (this == &src)
            return *this;

        this->~Deferred_array();
        
        std::copy(std::begin(src.m_dimension_boundary), std::end(src.m_dimension_boundary), std::begin(m_dimension_boundary));
    
        m_data = src.m_data;
        m_data_len = src.m_data_len;
        std::copy(std::begin(src.m_stride_size), std::end(src.m_stride_size), std::begin(m_stride_size));

        src.m_data = nullptr;
        src.m_data_len = 0 ;
        src.m_dimension_boundary = {};
        src.m_stride_size = {};

        return *this;
    }

    const element_t* data() const noexcept {return m_data;}
    const element_t* cdata() noexcept {return m_data;}
    element_t* data() noexcept {return m_data;}

    decltype(auto) operator[](int idx) noexcept 
    {
        decltype(auto) ret = Deferred_array<element_t, dimension>::Ptr_impl<dimension>{m_data, m_dimension_boundary, m_stride_size}[idx];
        return ret;
    }

    decltype(auto) operator[](int idx) const noexcept 
    {
        const Deferred_array<element_t, dimension>::Ptr_impl<dimension> t{m_data, m_dimension_boundary, m_stride_size};
        return t[idx];
    }

    template<std::size_t cnt>
    class Ptr_impl
    {
    private:
        element_t *m_ptr = nullptr;
        const std::array<std::size_t, dimension> &m_stride_size_arr;
        const std::array<std::size_t, dimension> &m_boundary_arr;


    public:
        Ptr_impl(element_t *ptr, 
                 const std::array<std::size_t, dimension> &boundary_arr,
                 const std::array<std::size_t, dimension> &stride_size_arr) 
        : m_ptr(ptr),
          m_stride_size_arr(stride_size_arr),
          m_boundary_arr(boundary_arr)
          {};

        const element_t& operator*() const noexcept {return *m_ptr;}
        element_t& operator*() noexcept {return *m_ptr;}
        const element_t* operator->() const noexcept {return m_ptr;}
        element_t* operator->() noexcept {return m_ptr;}

        bool is_out_of_rangge(std::size_t idx) const
        {
            auto x = m_boundary_arr[dimension - cnt];
            return x <= idx;
        }

        decltype(auto) operator[](std::size_t idx) noexcept
        {
            //assert(is_out_of_rangge(idx) == false);
            
            if constexpr (cnt > 1)
            {
                Deferred_array<element_t, dimension>::Ptr_impl<cnt - 1> 
                ret{m_ptr + idx*m_stride_size_arr[dimension - cnt], 
                    m_boundary_arr, 
                    m_stride_size_arr};
                
                return ret;
            }
            else
                return m_ptr[idx];
        }

        decltype(auto) operator[](std::size_t idx) const noexcept
        {
            //assert(is_out_of_rangge(idx) == false);
            if constexpr (cnt > 1)
            {
                const Deferred_array<element_t, dimension>::Ptr_impl<cnt - 1> 
                ret{m_ptr + idx*m_stride_size_arr[dimension-cnt], 
                    m_boundary_arr, 
                    m_stride_size_arr};
                return ret;
            }
            else
                return *(const_cast<const element_t*>(&m_ptr[idx]));                
        }

        friend Deferred_array<element_t, dimension>::Ptr_impl<cnt+1>;
    };

    void init_stride_size() noexcept
    {
        for(std::size_t i = 0, size = data_len() ; i < dimension ; i++)
        {
            m_stride_size[i] = size/m_dimension_boundary[i];
            size = m_stride_size[i];
        }
    }
    element_t *m_data;
    std::array<std::size_t, dimension> m_stride_size;
    std::size_t m_data_len;
    std::array<std::size_t, dimension> m_dimension_boundary;
};
