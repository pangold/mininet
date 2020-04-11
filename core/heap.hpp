#ifndef __HEAP_HPP__
#define __HEAP_HPP__

#include <cassert>
#include <vector>
#include <memory>

template<class T>
struct less_ {
    bool operator()(const T& left, const T& right) const { return left < right; }
};

template<class T>
struct greater_ {
    bool operator()(const T& left, const T& right) const { return left > right; }
};

template<class T>
struct less_ptr_ {
    bool operator()(const T& left, const T& right) const { return *left < *right; }
};

template<class T>
struct greater_ptr_ {
    bool operator()(const T& left, const T& right) const { return *left > *right; }
};

template<class T, class Compare = less_<T>>
class heap_type {
public:
    heap_type() = default;
    heap_type(T* data, size_t size)
    {
        heap_init__(data, size);
    }
public:
    void push(const T& element)
    {
        data_.push_back(element);
        heap_up__(data_.size() - 1);
    }
    T pop()
    {
        assert(!data_.empty());
        std::swap(data_.front(), data_.back());
        T element = data_.back();
        data_.pop_back();
        heap_down__(0);
        return std::move(element);
    }
    T& top()
    {
        assert(!data_.empty());
        return data_.front();
    }
    size_t size() const 
    { 
        return data_.size(); 
    }
    bool empty() const 
    { 
        return data_.empty(); 
    }
private:
    void heap_init__(T* data, size_t size)
    {
        data_.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            data_.push_back(data[i]);
        }
        for (int i = (data_.size() - 2) / 2; i >= 0; --i) {
            heap_down__(i);
        }
    }
    void heap_down__(int root)
    {
        Compare compare;
        int parent = root;
        size_t child = parent * 2 + 1;
        while (child < data_.size()) {
            if (child + 1 < data_.size() && compare(data_[child + 1], data_[child])) ++child;
            if (!compare(data_[child], data_[parent])) break;
            std::swap(data_[child], data_[parent]);
            parent = child;
            child = parent * 2 + 1;
        }
    }
    void heap_up__(int child)
    {
        Compare compare;
        int parent = (child - 1) / 2;
        while (parent >= 0) {
            if (!compare(data_[child], data_[parent])) break;
            std::swap(data_[parent], data_[child]);
            child = parent;
            parent = (child - 1) / 2;
        }
    }
private:
    std::vector<T> data_;
};

template <typename T> using min_heap = heap_type<T, less_<T>>;
template <typename T> using max_heap = heap_type<T, greater_<T>>;
template <typename T> using min_heap_ptr = heap_type<T, less_ptr_<T>>;
template <typename T> using max_heap_ptr = heap_type<T, greater_ptr_<T>>;

#endif 