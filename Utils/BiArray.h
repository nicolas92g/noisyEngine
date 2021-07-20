#pragma once
#include <glm/glm.hpp>
#include <Utils/utils.h>

#define TWO_DIM(i, j, imax) (i) + ((j) * imax)

namespace ns {
	template<typename T>
	class BiArray
	{
	public:
		BiArray(uint32_t x, uint32_t y);
		BiArray(const glm::ivec2& size);
		BiArray(uint32_t x, uint32_t y, T&& defaultValue);
		BiArray(const glm::ivec2& size, T&& defaultValue);
		~BiArray();

		size_t size() const;
		size_t x() const;
		size_t y() const;

		const T* data() const;
		T* data();

		void emplace(uint32_t x, uint32_t y, T&& newValue);

		T& value(uint32_t x, uint32_t y);
		const T& value(uint32_t x, uint32_t y) const;

		T& operator[](size_t pos);
		const T& operator[](size_t pos) const;

		void operator=(const BiArray& other);
		void operator=(const std::vector<T>& other);

		void to_vector(std::vector<T>& vector);
		size_t index(uint32_t x, uint32_t y) const;

		class Iterator {
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

		Iterator begin() {
			return Iterator(ptr_);
		}

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
		_STL_ASSERT(x != 0, "x's biarray is null");
		_STL_ASSERT(y != 0, "y's biarray is null");
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
		const size_t min = std::min(other.size_, size_);
		for (size_t i = 0; i < min; i++)
		{
			ptr_[i] = other.ptr_[i];
		}
	}
	template<typename T>
	inline void BiArray<T>::operator=(const std::vector<T>& other)
	{
		const size_t min = std::min(other.size(), size_);
		for (size_t i = 0; i < min; i++)
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

