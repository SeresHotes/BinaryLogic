
#include "helper.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <fstream>
using namespace std;


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
        list<T> arr;
    public:
        struct iterator {
        private:
            bool is_valid;
        public:
            list<T>::iterator iter;
            iterator() : is_valid(false), iter() {}
            iterator(list<T>::iterator iter) : is_valid(true), iter(iter) {}
            T& operator*() const noexcept {
                return *iter;
            }
            T* operator->() const noexcept {
                return &*iter;
            }

            bool operator==(const iterator& it) const noexcept {
                return it.iter == iter;
            }
            bool operator!=(const iterator& it) const noexcept {
                return iter != it.iter;
            }
            operator bool() {
                return is_valid;
            }
        };
        iterator begin() {
            return iterator(arr.begin());
        }
        iterator end() {
            return iterator(arr.end());
        }

        template<class ...Args>
        iterator alloc(Args&&... args) {
            arr.emplace_back(args...);
            auto t = arr.end();
            t--;
            return iterator(t);
        }
        void free(iterator it) {
            arr.erase(it.iter);
        }

    };

    

    container_wrapper<FunctionNode> alloc_arr;
public:
    int counter_all = 0;
    int counter_inv = 0;
    ostream &os;
    bool free_asp;
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
        int id;
        FunctionNode(BinaryFunction func, iterator parent, Origin origin, int id) :
            func(func), parent(parent), origin(origin), id(id) {

        }
        FunctionNode(BinaryFunction func,  Origin origin, int id) :
            func(func), origin(origin), id(id) {
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
        counter_all++;
        auto it = alloc_arr.alloc(func, parent, origin, counter_all);
        parent->childs.push_back(it);
        return it;
    }

    iterator _alloc(BinaryFunction func,  FunctionNode::Origin origin) {
        counter_all++;
        return alloc_arr.alloc(func, origin, counter_all);
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
            counter_inv++;
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

    void _save_node_data_and_free_childs(iterator node) {
        //cout << "delete " << node->id << endl;
        int id = node->id;
        int pid = node->parent ? node->parent->id : -1;
        os << "@ " << node->id << " " << node->func << " " << pid << " " << int(node->origin) << endl;
        for (auto& it : node->childs) {
            //cout << " " << it->id;
            os << it->id << " ";
            alloc_arr.free(it);
        }
        os << endl;
    }
    void _generateBinFuncTree(MyStack<iterator, VariableCount>& stack,
                              typename MyStack<iterator, VariableCount>::iterator invertionBegin) {
        static int counter = 0;
        counter++;
        if (counter % 100000 == 0) {
            cout << '@' << counter_all << " " << counter_inv << " " << counter_inv / float(counter_all) << endl;
        }
        auto old_end = stack.last();
        for (auto head = invertionBegin; head != stack.end(); ++head) {
            if (_tryInvertion(stack, (*head)->func)) {
                _union_last(stack, stack.begin());
                _generateBinFuncTree(stack, head);
                auto t = old_end;
                t;

                if (free_asp) {
                    for (auto it = stack.last(); it != t; --it) {
                        _save_node_data_and_free_childs(*it);
                    }
                }
                ++t;
                stack.erase(t, stack.end());
            }
        }
    }


    

public:
    BinaryFuncTree() : os(cout), free_asp(false) {}
    BinaryFuncTree(ostream &os) : os(os), free_asp(true) {}
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
        if (free_asp) {
            auto end = stack.begin();
            --end;
            for (auto it = stack.last(); it != end; --it) {
                _save_node_data_and_free_childs(*it);
            }
        }
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

class BufferedOstream {
    FILE* file;
    string buf;
    const int max_size;
public:
    BufferedOstream(FILE* file, int max_size) : file(file), max_size(max_size) {
        buf.reserve(max_size);
    }

    void write(const string &str) {
        if (str.size() + buf.size() > max_size) {
            fwrite(buf.data(), 1, buf.size(), file);
            buf.clear();
        } else {
            buf += str;
        }
    }
};

template<int VariableCount>
class BinaryFuncTree2 {
private:
    struct FunctionNode {
        enum struct Origin {
            UNION,
            INVERTION
        };
        BinaryFunction func;
        FunctionNode* parent;
        Origin origin;
        vector<FunctionNode*> childs;
        int id;
        FunctionNode(BinaryFunction func, FunctionNode* parent, Origin origin, int id) :
            func(func), parent(parent), origin(origin), id(id) {
            if (parent) {
                parent->childs.push_back(this);
            }
        }
        friend ostream& operator<<(ostream& os, const FunctionNode& node) {
            os << ((node.origin == Origin::UNION) ? 'U' : 'I');
            for (int i = (1 << VariableCount) - 1; i >= 0; i--) {
                os << (int(node.func >> i) & 1);
            }
            return os;
        }
    };
    ObjectPool<FunctionNode> functionPool;
    using iterator = FunctionNode*;

    iterator _alloc(BinaryFunction func, iterator parent, FunctionNode::Origin origin) {
        counter_all++;
        auto it = functionPool.alloc(func, parent, origin, counter_all);
        return it;
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
            counter_inv++;
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

    void _save_node_data_and_free_childs(iterator node) {
        //cout << "delete " << node->id << endl;
        char buf[3000] = { 0 };
        int id = node->id;
        int pid = node->parent ? node->parent->id : -1;

        int len = snprintf(buf, sizeof(buf), "@ %d %d %d %d\n", node->id, int(node->func), pid, int(node->origin));

        for (auto& it : node->childs) {
            //cout << " " << it->id;
            len += snprintf(buf + len, sizeof(buf) - len, "%d ", it->id);
            functionPool.free(it);
        }
        len += snprintf(buf + len, sizeof(buf) - len, "\n");

        os.write(string(buf));
    }
    void _generateBinFuncTree(MyStack<iterator, VariableCount>& functionStack) {
        std::list<typename MyStack<iterator, VariableCount>::iterator> depthStack;
        depthStack.push_back(functionStack.begin());
        depthStack.push_back(functionStack.begin());
        auto cur = &depthStack.back();

        unsigned int counter = 0;
        while (true) {
            counter++;
            if (counter % 2000000 == 0) {
                cout << '@' << counter_all << " " << counter_inv << " " << counter_inv / float(counter_all) << endl;
            }
            if (*cur == functionStack.end()) {
                depthStack.pop_back();

                auto start_to_delete = depthStack.back();
                if (free_asp) {
                    for (auto it = functionStack.last(); it != start_to_delete; --it) {
                        _save_node_data_and_free_childs(*it);
                    }
                    _save_node_data_and_free_childs(*start_to_delete);
                }
                functionStack.erase(start_to_delete, functionStack.end());
                depthStack.pop_back();


                if (depthStack.empty()) {
                    break;
                } else {
                    cur = &depthStack.back();
                    ++(*cur);
                    continue;
                }
            }
            if (_tryInvertion(functionStack, (**cur)->func)) {
                auto start_to_delete = functionStack.end();
                --start_to_delete;
                depthStack.push_back(start_to_delete);

                _union_last(functionStack, functionStack.begin());
                depthStack.push_back(*cur);
                cur = &depthStack.back();
            } else {
                ++(*cur);
            }

        }
    }
public:
    int counter_all = 0;
    int counter_inv = 0;
    BufferedOstream os;
    bool free_asp;
    BinaryFuncTree2() : os(stdout, 10000), free_asp(false) {}
    BinaryFuncTree2(FILE* file, int buffer_size = 10000) : os(file, buffer_size), free_asp(true) {}
    iterator tree_root = iterator();
    void generateBinFuncTree() {
        constexpr int N = VariableCount;
        MyStack<iterator, VariableCount> stack;

        auto root = _alloc(0, nullptr, FunctionNode::Origin::UNION);
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

        _generateBinFuncTree(stack);
        if (free_asp) {
            auto end = stack.begin();
            --end;
            for (auto it = stack.last(); it != end; --it) {
                _save_node_data_and_free_childs(*it);
            }
        }
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
    }
};


int main() {
    FILE* file;
    fopen_s(&file, "log.txt", "w");
    BinaryFuncTree2<3> bft(file, 1000000);
    bft.generateBinFuncTree();
    fclose(file);
    //bft.printBinFuncTree();
    return 0;
}