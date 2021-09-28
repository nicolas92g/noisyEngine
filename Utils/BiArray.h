#pragma once
#include <glm/glm.hpp>
#include <Utils/utils.h>

#define TWO_DIM(i, j, imax) (i) + ((j) * imax)

namespace ns {
	template<typename T>
	/**
	 * @brief biarray stand for bi-dimensional-array, the width and the height of the array are given in the constructor and then the array can't be resized.
	 * You can use it as a standard bi-dimensional array but it use a single 1-dimensional array to store all of the data so you can also use it as a standard 1 dim array which has a size of 
	 * width time height.
	 * the only dynamic heap allocation is in the constructor, all the others function are used to access and modify the array data
	 */
	class BiArray
	{
	public:
		/**
		 * @brief standard constructor, allocate memory in the heap
		 * the size of the allocation is equal to sizeof(T) * x * y bytes
		 * \param x width of the bi dim array 
		 * \param y height of the bi dim array 
		 */
		BiArray(uint32_t x, uint32_t y);
		/**
		 * @brief use a different input type but is the same as the standard constructor
		 * \param size width and height of the bi dim array
		 */
		BiArray(const glm::ivec2& size);
		/**
		 * @brief execute standard constructor and then initialize the array's values
		 * \param x	 width of the bi dim array 
		 * \param y	 height of the bi dim array 
		 * \param defaultValue value used to initialize array
		 */
		BiArray(uint32_t x, uint32_t y, T&& defaultValue);
		/**
		 * @brief same as before
		 * \param size
		 * \param defaultValue
		 */
		BiArray(const glm::ivec2& size, T&& defaultValue);
		/**
		 * @brief copy constructor
		 * \param other
		 */
		BiArray(const BiArray& other);
		/**
		 * @brief free the heap allocated memory
		 */
		~BiArray();
		/**
		 * @brief allow to know the number of elements that exist in the array
		 *( is equal to x * y )
		 * \return the array's size
		 */
		size_t size() const;
		/**
		 * \return the biarray's width
		 */
		size_t x() const;
		/**
		 * \return the biarray's height
		 */
		size_t y() const;
		/**
		 * @brief return a constant pointer to the single dimension array used to store the biarray data
		 * \return a constant T pointer
		 */
		const T* data() const;
		/**
		 * @brief return a pointer to the single dimension array used to store the biarray data
		 * \return a constant T pointer
		 */
		T* data();
		/**
		 * @brief allow to modify the bi dimensional array
		 * \param x index 1
		 * \param y index 2 
		 * \param newValue value to put at the location
		 */
		void emplace(uint32_t x, uint32_t y, T&& newValue);
		/**
		 * @brief return a reference to an element of the bi dimensional array
		 * \param x index 1 
		 * \param y index 2
		 * \return T reference
		 */
		T& value(uint32_t x, uint32_t y);
		/**
		 * @brief return a const reference to an element of the bi dimensional array
		 * \param x index 1
		 * \param y index 2
		 * \return const T reference
		 */
		const T& value(uint32_t x, uint32_t y) const;
		/**
		 * @brief allow to access the memory as a single dimension array 
		 * \param pos index
		 * \return T reference
		 */
		T& operator[](size_t pos);
		/**
		 * @brief allow to access the memory as a single dimension array
		 * \param pos index
		 * \return const T reference
		 */
		const T& operator[](size_t pos) const;
		/**
		 * @brief initialize the array with another
		 * \param other a biarray
		 */
		void operator=(const BiArray& other);
		/**
		 * @brief initialize the array with a std::vector<T>
		 * \param other a std::vector
		 */
		void operator=(const std::vector<T>& other);
		/**
		 * @brief put the data of this biarray into a std::vector<T>
		 * \param vector
		 */
		void to_vector(std::vector<T>& vector);
		/**
		 * @brief return the index of an element of the bi dimensional array 
		 * \param x index 1
		 * \param y index 2
		 * \return real memory index
		 */
		size_t index(uint32_t x, uint32_t y) const;
		/**
		 * @briefs store a pointer to an element of a biarray
		 * this Iterator class is only made to be able to use the for loop : for(const auto& element : biarrayName) 
		 */
		class Iterator 
		{
		public:
			Iterator(T* pointer);
			T& operator*();

			void operator+=(int add);
			void operator-=(int sub);
			void operator++();
			void operator--();
			bool operator!=(Iterator other);
			bool operator==(Iterator other);

		protected:
			T* ptr_;
		};
		/**
		 * @brief return a ptr_ to the first element 
		 * \return 
		 */
		Iterator begin() {
			return Iterator(ptr_);
		}
		/**
		 * @brief return a pointer to the last element + 1 (this ptr is of course invalid)
		 * \return 
		 */
		Iterator end() {
			return Iterator(ptr_ + size_);
		}
		
	protected:
		const size_t size_;
		const glm::uvec2 dim_;
		T* const ptr_;
	};


	//inline functions

	template<typename T>
	inline BiArray<T>::BiArray(uint32_t x, uint32_t y)
		:
		size_((size_t)x * (size_t)y),
		dim_(x, y),
		ptr_(new T[size_])
	{
#		ifndef NDEBUG
		_STL_ASSERT(x != 0, "x's biarray can't be zero");
		_STL_ASSERT(y != 0, "y's biarray can't be zero");
#		endif // !NDEBUG
	}

	template<typename T>
	inline BiArray<T>::BiArray(const glm::ivec2& size)
		: BiArray(size.x, size.y)
	{}

	template<typename T>
	inline BiArray<T>::BiArray(uint32_t x, uint32_t y, T&& defaultValue)
		:
		BiArray(x, y)
	{
		for (T& elem : *this)
			elem = defaultValue;
	}

	template<typename T>
	inline BiArray<T>::BiArray(const glm::ivec2& size, T&& defaultValue)
		: BiArray(size.x, size.y, std::move(defaultValue))
	{}

	template<typename T>
	inline BiArray<T>::BiArray(const BiArray& other)
		: BiArray(static_cast<uint32_t>(other.x()), static_cast<uint32_t>(other.y()))
	{
		*this = other;
	}

	template<typename T>
	inline BiArray<T>::~BiArray()
	{
		delete[] ptr_;
	}
	template<typename T>
	inline size_t BiArray<T>::size() const
	{
		return size_;
	}
	template<typename T>
	inline size_t BiArray<T>::x() const
	{
		return dim_.x;
	}
	template<typename T>
	inline size_t BiArray<T>::y() const
	{
		return dim_.y;
	}
	template<typename T>
	inline T* BiArray<T>::data()
	{
		return ptr_;
	}
	template<typename T>
	inline void BiArray<T>::emplace(uint32_t x, uint32_t y, T&& newValue)
	{
		ptr_[x + y * dim_.x] = newValue;
	}
	template<typename T>
	inline T& BiArray<T>::value(uint32_t x, uint32_t y)
	{
#		ifndef NDEBUG
		_STL_ASSERT(x < dim_.x, "BiArray first dimension out of range");
		_STL_ASSERT(y < dim_.y, "BiArray second dimension out of range");
#		endif

		return ptr_[x + y * dim_.x];
	}
	template<typename T>
	inline const T& BiArray<T>::value(uint32_t x, uint32_t y) const
	{
#		ifndef NDEBUG
		_STL_ASSERT(x < dim_.x, "BiArray first dimension out of range");
		_STL_ASSERT(y < dim_.y, "BiArray second dimension out of range");
#		endif

		return ptr_[x + y * dim_.x];
	}
	template<typename T>
	inline T& BiArray<T>::operator[](size_t pos)
	{
#		ifndef NDEBUG
		_STL_ASSERT(pos < size_, "BiArray out of range");
#		endif // !NDEBUG

		return ptr_[pos];
	}
	template<typename T>
	inline const T& BiArray<T>::operator[](size_t pos) const
	{
#		ifndef NDEBUG
		_STL_ASSERT(pos < size_, "BiArray out of range");
#		endif // !NDEBUG

		return ptr_[pos];
	}
	template<typename T>
	inline void BiArray<T>::operator=(const BiArray& other)
	{
		if (dim_ == other.dim_) {
			for (size_t i = 0; i < size_; i++)
			{
				ptr_[i] = other.ptr_[i];
			}
		}
		else {
			const uint32_t xmin = std::min(other.dim_.x, dim_.x);
			const uint32_t ymin = std::min(other.dim_.y, dim_.y);

			for (uint32_t i = 0; i < xmin; i++)
			{
				for (uint32_t j = 0; j < ymin; j++)
				{
					value(i, j) = other.value(i, j);
				}
			}
		}
	}
	template<typename T>
	inline void BiArray<T>::operator=(const std::vector<T>& other)
	{
#		ifndef NDEBUG
		_STL_VERIFY(other.size() == size_, "to copy a std::vector into a biarray, the vector must have the same size as the biarray");
#		endif // !NDEBUG

		for (size_t i = 0; i < size_; i++)
		{
			ptr_[i] = other[i];
		}
	}
	template<typename T>
	inline void BiArray<T>::to_vector(std::vector<T>& vector)
	{
		vector.resize(size_);
		for (size_t i = 0; i < size_; i++)
		{
			vector.emplace(vector.begin() + i, ptr_[i]);
		}
	}
	template<typename T>
	inline size_t BiArray<T>::index(uint32_t x, uint32_t y) const
	{
		return x + y * dim_.x;
	}
	template<typename T>
	inline const T* BiArray<T>::data() const
	{
		return ptr_;
	}

	template<typename T>
	inline BiArray<T>::Iterator::Iterator(T* pointer)
	{
		ptr_ = pointer;
	}

	template<typename T>
	inline T& BiArray<T>::Iterator::operator*()
	{
		return *ptr_;
	}

	template<typename T>
	inline void BiArray<T>::Iterator::operator+=(int add)
	{
		ptr_ += add;
	}

	template<typename T>
	inline void BiArray<T>::Iterator::operator-=(int sub)
	{
		ptr_ -= sub;
	}

	template<typename T>
	inline void BiArray<T>::Iterator::operator++()
	{
		ptr_ += 1;
	}

	template<typename T>
	inline void BiArray<T>::Iterator::operator--()
	{
		ptr_--;
	}

	template<typename T>
	inline bool BiArray<T>::Iterator::operator!=(Iterator other)
	{
		return ptr_ != other.ptr_;
	}

	template<typename T>
	inline bool BiArray<T>::Iterator::operator==(Iterator other)
	{
		return ptr_ == other.ptr_;
	}

}

