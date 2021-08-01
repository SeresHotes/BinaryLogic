#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <list>

using namespace std;

class VerticalPrint {
    vector<stringstream> arr;
    vector<char> filler;
    int base_add = 0;
public:
    int addhline() {
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
            cout << str.str() << endl;
        }
        arr.clear();
    }

};


template<int VariableCount>
class BinaryFuncTree {
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
            bool operator==(const iterator &it) const noexcept {
                return index == it.index;
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


private:
    template<int VariableCount>
    struct FunctionNode;
    typedef uint64_t BinaryFunction;
    using iterator = typename container_wrapper<FunctionNode<VariableCount>>::iterator;

    container_wrapper<FunctionNode<VariableCount>> alloc_arr;
public:
    int counter = 0;
private:

    template<int VariableCount>
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
        friend ostream& operator<<(ostream& os, const FunctionNode<VariableCount>& node) {
            os << ((node.origin == Origin::UNION) ? 'U' : 'I');
            for (int i = (1 << VariableCount) - 1; i >= 0; i--) {
                os << (int(node.func >> i) & 1);
            }
            return os;
        }
    };

    iterator _alloc(BinaryFunction func, iterator parent, FunctionNode<VariableCount>::Origin origin) {
        auto it = alloc_arr.alloc(func, parent, origin);
        parent->childs.push_back(it);
        return it;
    }

    iterator _alloc(BinaryFunction func,  FunctionNode<VariableCount>::Origin origin) {
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

    bool _tryInvertion(iterator& end, BinaryFunction value) {
        BinaryFunction val = (~value) & get_mask();
        if (find(end, val) == 0) {
            end = _alloc(val, end, FunctionNode<VariableCount>::Origin::INVERTION);
            return true;
        } else {
            return false;
        }

    }
    bool _tryUnion(iterator& end, BinaryFunction left, BinaryFunction right) {
        BinaryFunction val = left | right;
        if (find(end, val) == 0) {
            end = _alloc(val, end, FunctionNode<VariableCount>::Origin::UNION);
            return true;
        } else {
            return false;
        }
    }

    void _union_all(list<iterator>& stack,
                    const typename list<iterator>::iterator& unionBegin) {
        for (auto it = unionBegin; it != stack.end(); it++) {
            for (auto it2 = std::next(it); it2 != stack.end(); it2++) {
                auto end = stack.back();
                if (_tryUnion(end, (*it)->func, (*it2)->func)) {
                    stack.push_back(end);
                }
            }
        }
    }
    void _union_last(list<iterator>& stack,
                    const typename list<iterator>::iterator& unionBegin) {
        auto& val = stack.back();
        auto end = std::prev(stack.end());
        for (auto it = unionBegin; it != end; it++) {
            auto end = stack.back();
            if (_tryUnion(end, (*it)->func, val->func)) {
                stack.push_back(end);
            }
        }
    }


    void _generateBinFuncTree(list<iterator> &stack,
                              typename list<iterator>::iterator invertionBegin) {
        counter++;
        if (counter % 1000 == 0) {
            cout << '@' << counter << endl;
        }
        auto old_end = std::prev(stack.end());
        for (auto head = invertionBegin; head != stack.end(); head++) {
            auto temp = stack.back();
            if (_tryInvertion(temp, (*head)->func)) {
                stack.push_back(temp);
                _union_last(stack, stack.begin());
                _generateBinFuncTree(stack, head);
                stack.erase(std::next(old_end), stack.end());
            }
        }

    }

public:
    iterator tree_root;
    void generateBinFuncTree() {
        constexpr int N = VariableCount;
        list<iterator> stack;

        auto root = _alloc(0, FunctionNode<VariableCount>::Origin::UNION);
        auto head = root;
        stack.push_back(head);
        head = _alloc((~0) & get_mask(), head, FunctionNode<VariableCount>::Origin::UNION);
        stack.push_back(head);


        for (int i = 0; i < N; i++) {
            BinaryFunction func = 0;
            for (int j = 0; j < 1 << N; j++) {
                if (j & (1 << i)) {
                    func = func | (BinaryFunction(1) << j);
                }
            }
            head = _alloc(func, head, FunctionNode<VariableCount>::Origin::UNION);
            stack.push_back(head);
        }
        _union_all(stack, stack.begin());

        _generateBinFuncTree(stack, stack.begin());
        tree_root = root;
    }

    void printBinFuncTree() {
        VerticalPrint vp;
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
    BinaryFuncTree<2> bft;
    bft.generateBinFuncTree();
    bft.printBinFuncTree();
    cout << bft.counter << endl;
    return 0;
}