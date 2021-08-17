#pragma once
#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <list>

using namespace std;

/*
* Can have as many digits as needed
*/
typedef uint64_t BinaryFunction;

/*
* Optimized stack. We can have up to 2^2^VariableCount functions. For low 
* counts we can just have an array of all the functions and use to answer
* the question: do we already have this function in our stack?
*/
template<class T, int VariableCount>
class MyStack {
private:
    struct Node {
        T val;
        Node* prev;
        Node* next;
        bool is_valid;
        Node(int index, T val, Node* prev, Node* next)
            : val(val), prev(prev), next(next), is_valid(true) {

        }
        Node()
            : val(), prev(0), next(0), is_valid(false) {

        }
    };
    constexpr static int size = 1 << (BinaryFunction(1) << VariableCount);
    Node arr[size];

    Node _end;
    Node _rend;
public:
    MyStack() : _end(size, T(), &_rend, (Node*) 0), _rend(size, T(), (Node*) 0, &_end) {

    }
    struct iterator {

    public:
        Node* node;
        iterator() : node(0) {}
        iterator(Node* node) : node(node) {}
        T& operator*() const noexcept {
            return node->val;
        }
        T* operator->() const noexcept {
            return &node->val;
        }
        iterator& operator++() noexcept {
            node = node->next;
            return *this;
        }
        iterator& operator--() noexcept {
            node = node->prev;
            return *this;
        }
        bool operator==(const iterator& it) const noexcept {
            return node == it.node;
        }
    };
    iterator end() {
        iterator t = iterator(&_end);
        return t;
    }
    iterator rend() {
        iterator t = iterator(&_rend);
        return t;
    }
    iterator begin() {
        return iterator(_rend.next);
    }
    iterator last() {
        return iterator(_end.prev);
    }
    void push_back(const T& obj, BinaryFunction func) {
        auto t = _end.prev;
        arr[func].val = obj;
        arr[func].prev = t;
        arr[func].next = &_end;
        arr[func].is_valid = true;
        t->next = &arr[func];
        _end.prev = &arr[func];
    }
    void pop_back() {
        cout << "nya" << endl;
        _end.prev->is_valid = false;
        auto t = _end.prev->prev;
        _end.prev = t;
        t->next = &_end;
    }
    iterator find(BinaryFunction func) {
        if (arr[func].is_valid) {
            return iterator(&arr[func]);
        } else {
            return end();
        }
    }
    void erase(iterator begin, iterator end) {
        for (auto it = begin; it != end; ++it) {
            it.node->is_valid = false;
        }
        auto t = begin;
        --t;
        end.node->prev = t.node;
        t.node->next = end.node;
    }
};

namespace std {
    template<class T, int VariableCount>
    typename MyStack<T, VariableCount>::iterator
        prev(typename MyStack<T, VariableCount>::iterator it) {
        auto t = it--;
        return t;
    }
    template<class T, int VariableCount>
    MyStack<T, VariableCount>::iterator next(typename MyStack<T, VariableCount>::iterator it) {
        auto t = it++;
        return t;
    }
}

/*
* Fancy printing of tree
*/
class VerticalPrint {
    vector<stringstream> arr;
    vector<char> filler;
    size_t base_add = 0;
    ostream& os;
public:
    VerticalPrint() : os(cout) {}
    VerticalPrint(ostream& os) : os(os) {}

    size_t addhline() {
        filler.push_back(' ');
        arr.emplace_back();
        for (int i = 0; i < base_add; i++) {
            arr.back() << ' ';
        }
        return arr.size() - 1;
    }
    void endvline() {
        size_t count = 0;
        for (const auto& str : arr) {
            count = max(str.str().size(), count);
        }
        base_add = count;
        for (size_t i = 0; i < arr.size(); i++) {
            auto& str = arr[i];
            auto f = filler[i];
            for (size_t i = str.str().size(); i < count; i++) {
                str << f;
            }
        }
    }
    template<class T>
    void add(size_t line, const T& obj) {
        arr[line] << obj;
    }
    template<class T>
    void addsafe(size_t line, const T& obj) {
        while (line >= arr.size()) {
            addhline();
        }
        arr[line] << obj;
    }
    void setfiller(size_t line, char filler) {
        while (line >= arr.size()) {
            addhline();
        }
        this->filler[line] = filler;
    }
    char getfiller(size_t line) {
        return this->filler[line];
    }
    void resetfiller() {
        for (auto& f : filler) {
            f = ' ';
        }
    }


    void print() {
        for (const auto& str : arr) {
            const auto& s = str.str();
            for (int j = 0; j < s.size(); j++) {
                os << s[j];
            }
            os << endl;
            //cout << str.str() << endl;
        }
        arr.clear();
    }

};

void testVerticalPrint() {
    VerticalPrint vp;
    vp.addhline();
    vp.addhline();
    vp.addhline();
    vp.add(0, "hello ");
    vp.add(1, "w o r l d");
    vp.add(0, "?");
    vp.endvline();
    vp.add(0, "noooo");
    vp.add(1, 7483028);
    vp.add(2, "What??!!");
    vp.print();
}


/*
* Fast object allocation
*/
template<class T>
class ObjectPool {
public:
    std::list<T*> resources;
public:
    template<class ...Args>
    T* alloc(Args ...args) {
        if (resources.empty()) {
            return new T(args...);
        } else {
            T* res = resources.back();
            resources.pop_back();
            new (res) T(args...);
            return res;
        }
    }
    void free(T* obj) {
        resources.push_back(obj);
        obj->~T();
    }
};
/*
* Buffering saving to disk to maximize speed
*/
class BufferedOstream {
    ostream& os;
    uint8_t* buf;
    int size = 0;
    const int max_size;
public:
    BufferedOstream(ostream& os, int max_size) : os(os), max_size(max_size) {
        buf = new uint8_t[max_size];
    }
    ~BufferedOstream() {
        delete[] buf;
    }
    void write(uint8_t* data, size_t size) {
        if (size + this->size > max_size) {
            os.write(reinterpret_cast<const char*>(buf), this->size);
            os.write(reinterpret_cast<const char*>(data), size);
            this->size = 0;
        } else {
            memcpy_s(buf + this->size, max_size - this->size, data, size);
            this->size += size;
        }
    }
    void fflush() {
        os.write(reinterpret_cast<const char*>(buf), size);
    }
};
