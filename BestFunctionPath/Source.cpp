#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <list>

using namespace std;

typedef uint64_t BinaryFunction;

template<int VariableCount>
struct FunctionNode {
    enum struct Origin {
        UNION,
        INVERTION
    };
    BinaryFunction func;
    FunctionNode* parent;
    Origin origin;
    vector<FunctionNode*> childs;
    FunctionNode(BinaryFunction func, FunctionNode* parent, Origin origin) : 
            func(func), parent(parent), origin(origin) {
        if (parent) {
            parent->childs.push_back(this);
        }
    }
    friend ostream& operator<<(ostream& os, const FunctionNode<VariableCount>& node) {
        os << ((node.origin == Origin::UNION) ? 'U' : 'I');
        for (int i = (1 << VariableCount) - 1; i >= 0; i--) {
            os << (int(node.func >> i) & 1);
        }
        return os;
    }
};

template<int VariableCount>
FunctionNode<VariableCount>* find(FunctionNode<VariableCount>* end, BinaryFunction func) {
    while (end) {
        if (end->func == func) {
            return end;
        }
        end = end->parent;
    }
    return 0;
}

template<int VariableCount>
BinaryFunction get_mask() {
    BinaryFunction mask = 0;
    for (int i = 0; i < 1 << VariableCount; i++) {
        mask <<= 1;
        mask |= 1;
    }
    return mask;
}

template<int VariableCount>
bool _tryInvertion(FunctionNode<VariableCount>*& end, BinaryFunction value) {
    BinaryFunction val = (~value) & get_mask<VariableCount>();
    if (find(end, val) == 0) {
        auto temp = new FunctionNode<VariableCount>(val, end, FunctionNode<VariableCount>::Origin::INVERTION);
        end = temp;
        return true;
    } else {
        return false;
    }

}
template<int VariableCount>
bool _tryUnion(FunctionNode<VariableCount>*& end, BinaryFunction left, BinaryFunction right) {
    BinaryFunction val = left | right;
    if (find(end, val) == 0) {
        auto temp = new FunctionNode<VariableCount>(val, end, FunctionNode<VariableCount>::Origin::UNION);
        end = temp;
        return true;
    } else {
        return false;
    }
}
static int counter = 0;
template<int VariableCount>
void _generateBinFuncTree(FunctionNode<VariableCount>* unionBegin,
                          FunctionNode<VariableCount>* invertionBegin, 
                          FunctionNode<VariableCount>* end) {
    counter++;
    if (counter % 10 == 0) {
        cout << '@' << counter << endl;
    }
    list<FunctionNode<VariableCount>*> stack;
    for (auto head = end; head != unionBegin; head = head->parent) {
        stack.push_front(head);
    }
    stack.push_front(unionBegin);
    for (auto head = stack.begin(); head != stack.end(); head++) {
        auto head2 = head;
        head2++;
        for (; head2 != stack.end(); head2++) {
            if (_tryUnion(end, (*head)->func, (*head2)->func)) {
                stack.push_back(end);
            }
        }
    }
    auto head = stack.begin();
    while (*head != invertionBegin) {
        head++;
    }
    for (; head != stack.end(); head++) {
        auto temp = end;
        if (_tryInvertion(temp, (*head)->func)) {
            _generateBinFuncTree(unionBegin, *head, temp);
        }
    }
    
}

template<int VariableCount>
FunctionNode<VariableCount>* generateBinFuncTree() {
    constexpr int N = VariableCount;
    auto root = new FunctionNode<VariableCount>(0, nullptr, FunctionNode<VariableCount>::Origin::UNION);
    auto head = root;
    head = new FunctionNode<VariableCount>((~0) & get_mask<VariableCount>(), head, FunctionNode<VariableCount>::Origin::UNION);


    for (int i = 0; i < N; i++) {
        BinaryFunction func = 0;
        for (int j = 0; j < 1 << N; j++) {
            if (j & (1 << i)) {
                func = func | (BinaryFunction(1) << j);
            }
        }
        head = new FunctionNode<VariableCount>(func, head, FunctionNode<VariableCount>::Origin::UNION);
    }
    _generateBinFuncTree(root, root, head);


    return root;
}


template<int VariableCount>
class BinaryFuncTree {
public:
    typedef uint64_t BinaryFunction;

    template<int VariableCount>
    struct FunctionNode {
        enum struct Origin {
            UNION,
            INVERTION
        };
        BinaryFunction func;
        FunctionNode* parent;
        Origin origin;
        vector<FunctionNode*> childs;
        FunctionNode(BinaryFunction func, FunctionNode* parent, Origin origin) :
            func(func), parent(parent), origin(origin) {
            if (parent) {
                parent->childs.push_back(this);
            }
        }
        friend ostream& operator<<(ostream& os, const FunctionNode<VariableCount>& node) {
            os << ((node.origin == Origin::UNION) ? 'U' : 'I');
            for (int i = (1 << VariableCount) - 1; i >= 0; i--) {
                os << (int(node.func >> i) & 1);
            }
            return os;
        }
    };

private:
    vector<FunctionNode<VariableCount>> alloc_arr;
    int counter = 0;

    FunctionNode<VariableCount>* find(FunctionNode<VariableCount>* end, BinaryFunction func) {
        while (end) {
            if (end->func == func) {
                return end;
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

    bool _tryInvertion(FunctionNode<VariableCount>*& end, BinaryFunction value) {
        BinaryFunction val = (~value) & get_mask();
        if (find(end, val) == 0) {
            end = &alloc_arr.emplace_back(val, end, FunctionNode<VariableCount>::Origin::INVERTION);
            return true;
        } else {
            return false;
        }

    }
    bool _tryUnion(FunctionNode<VariableCount>*& end, BinaryFunction left, BinaryFunction right) {
        BinaryFunction val = left | right;
        if (find(end, val) == 0) {
            end = &alloc_arr.emplace_back(val, end, FunctionNode<VariableCount>::Origin::UNION);
            return true;
        } else {
            return false;
        }
    }
    void _generateBinFuncTree(FunctionNode<VariableCount>* unionBegin,
                              FunctionNode<VariableCount>* invertionBegin,
                              FunctionNode<VariableCount>* end) {
        counter++;
        if (counter % 10 == 0) {
            cout << '@' << counter << endl;
        }
        list<FunctionNode<VariableCount>*> stack;
        for (auto head = end; head != unionBegin; head = head->parent) {
            stack.push_front(head);
        }
        stack.push_front(unionBegin);
        for (auto head = stack.begin(); head != stack.end(); head++) {
            auto head2 = head;
            head2++;
            for (; head2 != stack.end(); head2++) {
                if (_tryUnion(end, (*head)->func, (*head2)->func)) {
                    stack.push_back(end);
                }
            }
        }
        auto head = stack.begin();
        while (*head != invertionBegin) {
            head++;
        }
        for (; head != stack.end(); head++) {
            auto temp = end;
            if (_tryInvertion(temp, (*head)->func)) {
                _generateBinFuncTree(unionBegin, *head, temp);
            }
        }

    }

public:

    FunctionNode<VariableCount>* generateBinFuncTree() {
        constexpr int N = VariableCount;
        FunctionNode<VariableCount>* root = &alloc_arr.emplace_back(0, nullptr, FunctionNode<VariableCount>::Origin::UNION);
        auto head = root;
        head = &alloc_arr.emplace_back((~0) & get_mask(), head, FunctionNode<VariableCount>::Origin::UNION);


        for (int i = 0; i < N; i++) {
            BinaryFunction func = 0;
            for (int j = 0; j < 1 << N; j++) {
                if (j & (1 << i)) {
                    func = func | (BinaryFunction(1) << j);
                }
            }
            head = &alloc_arr.emplace_back(func, head, FunctionNode<VariableCount>::Origin::UNION);
        }
        //_generateBinFuncTree(root, root, head);


        return root;
    }
};

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
void printBinFuncTree(FunctionNode<VariableCount>* node) {
    VerticalPrint vp;
    list<pair<FunctionNode<VariableCount>*, size_t>> stack;

    auto head = node;
    while (true) {
        while (head) {
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
                head = nullptr;
            }
        }
        vp.endvline();

        while (!stack.empty()) {
            auto &t = stack.back();
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

int main() {

    BinaryFuncTree<2> bft;
    auto tree = bft.generateBinFuncTree();
    //auto tree = generateBinFuncTree<3>();
    printBinFuncTree(tree);
    return 0;
}