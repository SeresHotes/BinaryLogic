#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <list>
#include <fstream>

using namespace std;

typedef uint64_t BinaryFunction;

template<class T, int VariableCount>
class MyStack {
private:
    struct Node {
        int index;
        T val;
        Node* prev;
        Node* next;
        bool is_valid;
        Node(int index, T val, Node* prev, Node* next)
            : index(index), val(val), prev(prev), next(next), is_valid(true) {

        }
        Node()
            : index(0), val(), prev(0), next(0), is_valid(false){

        }
    };
    constexpr static int size = 1 << (BinaryFunction(1) << VariableCount);
    Node arr[size];

    Node _end;
    Node _rend;
public:
    MyStack() : _end(size, T(), &_rend, (Node*)0), _rend(size, T(), (Node*) 0, &_end) {

    }
    struct iterator {

    public:
        Node* node;
        iterator() : node(0) {}
        iterator(Node *node) : node(node) {}
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
    iterator end()  {
        iterator t = iterator(&_end);
        return t;
    }
    iterator begin()  {
        return iterator(_rend.next);
    }
    iterator last()  {
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
        auto t = _end.prev->prev;
        _end.prev = t;
        _end.prev->is_valid = false;
        t->next = _end.prev;
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

class VerticalPrint {
    vector<stringstream> arr;
    vector<char> filler;
    size_t base_add = 0;
    ostream& const os;
public:
    VerticalPrint() : os(cout) {}
    VerticalPrint(ostream &os) : os(os) {}

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


template<int VariableCount>
class BinaryFuncTree {

private:
    template<class T>
    struct container_wrapper;
    struct FunctionNode;
    using iterator = typename container_wrapper<FunctionNode>::iterator;


    template<class T>
    struct container_wrapper {
    private:
        vector<T> arr;
    public:
        struct iterator {
        private:
            vector<T>* arr;
            int index;
        public:
            iterator() : arr(0), index(0) {}
            iterator(vector<T>* arr, int index) : arr(arr), index(index) {}
            T& operator*() const noexcept {
                return (*arr)[index];
            }
            T* operator->() const noexcept {
                return &(*arr)[index];
            }
            iterator operator+(int a) const noexcept  {
                return iterator{ index + a };
            }
            iterator operator-(int a) const noexcept  {
                return iterator{ index - a };
            }
            bool operator==(const iterator& it) const noexcept {
                return index == it.index;
            }
            bool operator!=(const iterator& it) const noexcept {
                return index != it.index;
            }
            operator bool() const noexcept {
                return arr != 0 && index >= 0 && index < arr->size();
            }

        };
        iterator begin() {
            return iterator(&arr, 0);
        }
        iterator end() {
            return iterator(&arr, arr.size());
        }

        template<class ...Args>
        iterator alloc(Args&&... args) {
            arr.emplace_back(args...);
            return iterator(&arr, arr.size() - 1);
        }

    };

    

    container_wrapper<FunctionNode> alloc_arr;
public:
    int counter = 0;
private:

    struct FunctionNode {
        enum struct Origin {
            UNION,
            INVERTION
        };
        BinaryFunction func;
        iterator parent;
        Origin origin;
        vector<iterator> childs;
        FunctionNode(BinaryFunction func, iterator parent, Origin origin) :
            func(func), parent(parent), origin(origin) {

        }
        FunctionNode(BinaryFunction func,  Origin origin) :
            func(func), origin(origin) {
        }
        friend ostream& operator<<(ostream& os, const FunctionNode& node) {
            os << ((node.origin == Origin::UNION) ? 'U' : 'I');
            for (int i = (1 << VariableCount) - 1; i >= 0; i--) {
                os << (int(node.func >> i) & 1);
            }
            return os;
        }
    };

    iterator _alloc(BinaryFunction func, iterator parent, FunctionNode::Origin origin) {
        auto it = alloc_arr.alloc(func, parent, origin);
        parent->childs.push_back(it);
        return it;
    }

    iterator _alloc(BinaryFunction func,  FunctionNode::Origin origin) {
        return alloc_arr.alloc(func, origin);
    }

    iterator* find(iterator end, BinaryFunction func) {
        while (end) {
            if (end->func == func) {
                return &end;
            }
            end = end->parent;
        }
        return 0;
    }

    BinaryFunction get_mask() {
        BinaryFunction mask = 0;
        for (int i = 0; i < 1 << VariableCount; i++) {
            mask <<= 1;
            mask |= 1;
        }
        return mask;
    }

    bool _tryInvertion(MyStack<iterator, VariableCount>& stack, BinaryFunction value) {
        BinaryFunction val = (~value) & get_mask();
        if (stack.find(val) == stack.end()) {
            auto t = _alloc(val, *stack.last(), FunctionNode::Origin::INVERTION);
            stack.push_back(t, t->func);
            return true;
        } else {
            return false;
        }

    }
    bool _tryUnion(MyStack<iterator, VariableCount>& stack, BinaryFunction left, BinaryFunction right) {
        BinaryFunction val = left | right;
        if (stack.find(val) == stack.end()) {
            auto t = _alloc(val, *stack.last(), FunctionNode::Origin::UNION);
            stack.push_back(t, t->func);
            return true;
        } else {
            return false;
        }
    }

    void _union_all(MyStack<iterator, VariableCount>& stack,
                    const typename MyStack<iterator, VariableCount>::iterator& unionBegin) {
        for (auto it = unionBegin; it != stack.end(); ++it) {
            auto it2 = it;
            ++it2;
            for (; it2 != stack.end(); ++it2) {
                _tryUnion(stack, (*it)->func, (*it2)->func);
            }
        }
    }
    void _union_last(MyStack<iterator, VariableCount>& stack,
                    const typename MyStack<iterator, VariableCount>::iterator& unionBegin) {
        auto& val = *stack.last();
        auto end = stack.last();
        for (auto it = unionBegin; it != end; ++it) {
            _tryUnion(stack, (*it)->func, val->func);
        }
    }


    void _generateBinFuncTree(MyStack<iterator, VariableCount> &stack,
                              typename MyStack<iterator, VariableCount>::iterator invertionBegin) {
        counter++;
        if (counter % 100000 == 0) {
            cout << '@' << counter << endl;
        }
        auto old_end = stack.last();
        for (auto head = invertionBegin; head != stack.end(); ++head) {
            if (_tryInvertion(stack, (*head)->func)) {
                _union_last(stack, stack.begin());
                _generateBinFuncTree(stack, head);
                auto t = old_end;
                ++t;
                stack.erase(t, stack.end());
            }
        }

    }

public:
    iterator tree_root;
    void generateBinFuncTree() {
        constexpr int N = VariableCount;
        MyStack<iterator, VariableCount> stack;

        auto root = _alloc(0, FunctionNode::Origin::UNION);
        auto head = root;
        stack.push_back(head, head->func);
        head = _alloc((~0) & get_mask(), head, FunctionNode::Origin::UNION);
        stack.push_back(head, head->func);


        for (int i = 0; i < N; i++) {
            BinaryFunction func = 0;
            for (int j = 0; j < 1 << N; j++) {
                if (j & (1 << i)) {
                    func = func | (BinaryFunction(1) << j);
                }
            }
            head = _alloc(func, head, FunctionNode::Origin::UNION);
            stack.push_back(head, head->func);
        }
        _union_all(stack, stack.begin());

        _generateBinFuncTree(stack, stack.begin());
        tree_root = root;
    }

    void printBinFuncTree() {
        ofstream fout;
        fout.open("result.txt", ios::out);
        VerticalPrint vp(fout);
        list<pair<iterator, size_t>> stack;

        auto head = tree_root;
        while (true) {
            while (true) {
                vp.addsafe(stack.size(), *head);
                vp.addsafe(stack.size(), vp.getfiller(stack.size()));
                if (head->childs.size() > 0) {
                    if (head->childs.size() > 1) {
                        vp.setfiller(stack.size() + 1, '-');
                    } else {
                        vp.setfiller(stack.size() + 1, ' ');
                    }
                    stack.push_back(make_pair(head, 1));
                    head = head->childs[0];
                } else {
                    break;
                }
            }
            vp.endvline();

            while (!stack.empty()) {
                auto& t = stack.back();
                if (t.first->childs.size() > t.second) {
                    if (t.first->childs.size() - t.second > 1) {
                        vp.setfiller(stack.size(), '-');
                    } else {
                        vp.setfiller(stack.size(), ' ');
                    }
                    head = t.first->childs[t.second];
                    t.second++;
                    break;
                } else {
                    vp.setfiller(stack.size(), ' ');
                    stack.pop_back();
                }
            }
            if (stack.empty()) {
                break;
            }
        }
        vp.print();
        fout.close();

        /*stack.push_back(make_pair(node, 0));

        int depth = 0;
        while (!stack.empty()) {
            for (; !stack.empty();) {
                auto head = stack.front();
                if (head.second != depth) {
                    break;
                } else {
                    cout << *head.first << " ";
                    for (const auto& v : head.first->childs) {
                        stack.push_back(make_pair(v, depth + 1));
                    }
                }
                stack.pop_front();
            }
            cout << endl;
            depth++;
        }*/

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
struct pointer {
    typename vector<int>::iterator iter;
    bool is_valid;

};
int main() {
    BinaryFuncTree<3> bft;
    bft.generateBinFuncTree();
    bft.printBinFuncTree();
    cout << bft.counter << endl;
    return 0;
}