/*
 * Reasoning
 * 
 *   I want to create the fastest possible computer in Minecraft.
 * Unions of signals are very simple and gives 0 lattency.
 * But inversions create a lot of lattency. To create fast 
 * logic curcuit I need to minimize amount of
 * inversions. One of the solutions is to broad force it 
 * and try all the different variants of inversions and find the
 * best way to invert functions and unite them to get the
 * desired one. For example, if we want to get XOR of x1 and x2,
 * we can use ~(x1||~x2)||~(~x1||x2) which gives 4 inversions.
 * But there is better sollution: ~(x1||~[x1||x2])||~(x2||~[x1||x2])
 * which gives 3 inversions, as we can reuse the signal ~[x1||x2]! 
 * 
 * 
 * The algorithm
 *
 *   This program creates tree of binary functions. It starts
 * from basic ones like 0, 1 x1, x2, ..., xn and creates new
 * ones based on the following algorithm:
 * 1) Union is free. So, we can try every union of already 
 * existing functions.
 * 2) Inverstion costs something. When all unions are tested
 * we try create new function using inversion of only one of 
 * existing ones. If we succeed, then we repeat from stage 1.
 * 3) If there are no function can be created through inversions
 * than we roll back our changes to set of functions and try
 * different order of inversions. Thus, this brunch of the tree
 * is finished. If different order of inversions exists, then
 * we continue from stage 1 again.
 * 4) If any inversion order can't get new branches to the tree,
 * then the tree is finished.
 * 
 *   Function are represented as vector values. For example of
 * 2 variable, x1 is shown as 1010, x2 - as 1100, ~x1||x2 = 1101
 * and so on. For 3 variables we have 8 digit binary numbers
 * and for N variable we have 2^N digit binary numbers. This
 * representation allows us make simple inversion and unions:
 * ~x1 = ~1010 = 0101 
 * x1||x2 = 1010 | 1100 = 1110
 * 
 * 
 * Example of the algorithm for 2 variables:
 * 0. We have basic functions:
 * 0000
 * 1111
 * 1010
 * 1100
 * 1. After stage 1 we create:
 * 1110
 * 2. Now stage 2. We can't get any new through unions, so we 
 * have to make inversions. The first number that gives us new 
 * number is 1010, so:
 * 0101
 * 3. Again stage 1.After inversion there is possibility that we 
 * can get new functions through unions. They are free, so we can 
 * do as many unions as we want:
 * 1101
 * 4. Stage 2. Next possible inversion is 1100:
 * 0011
 * 5. Stage 1:
 * 1011
 * 0111
 * 6. Stage 2:
 * 0001
 * 7. Stage 1: no new unions. 
 * 8. Stage 2. Remember, we can get try use inversion on any number
 * of allready created list. Right now we try the first number, that
 * can get us new one. Now it is 1101:
 * 0010
 * 9. Stage 1: no new unions.
 * 10. Stage 2: 
 * 0100
 * 11. Stage 1:
 * 0110
 * 12. Stage 2:
 * 1000
 * 13. Stage 1:
 * 1001
 * 14. Stage 2: no new inversions. Here we are. There are no new 
 * numbers/functions can be created through unions and inversions.
 * Actually, as we used x1 and x2 as based functions, then we get
 * all of the functions of two variables. If you count you get 16
 * functions. For N variables we will get 2^2^N functions.
 *   Now we can try any other inversion order. The simplest way to
 * traverse all the orders of this tree is to use DFS. We just roll
 * back up to the latest inversion to step 11.
 *   On step 11 we've got 0110. We tried inversion of 0001. The next
 * viable and the only choise is inversion of 0110:
 * 15. Stage 2:
 * 1001
 * 16. Stage 1: no new unions
 * We could try again inversion of 0001 and get all the functions, but
 * it is simillar to the first branch. So, we can just stop here and 
 * go futher using DFS.
 * 
 * The end result of the tree is this:
 * U0000                                                                                                                                     
 * U1111                                                                                                                                     
 * U1010
 * U1100
 * U1110
 * I0101-------------------------------------------------------------------------------------I0011-------------------------I0001
 * U1101                                                                                     U1011                         U1011
 * I0011-------------------------------------------------------I0001-------------------I0010 I0001-------------------I0100 U1101
 * U1011                                                       U1011                   U0111 U1101                   U0111 I0100-------I0010
 * U0111                                                       I0010-------------I0100 I1000 I0100-------------I0010 I1000 U0101       U0011
 * I0001-------------------------I0010-------------I0100-I1000 U0111                         U0111                         I0010
 * I0010-------------I0100-I1000 I0100-------I1000 I1000       U0011                         U0101                         U0011
 * I0100-------I1000 I1000 U1001 U0110                         I0100-------I1000             I0010-------I1000             U0110
 * U0110       U1001 U1001 I0110 I1000-I1001                   U0110       U1001             U0110       U1001             U0111
 * I1000-I1001 I0110 I0110       I1001                         I1000-I1001 I0110             I1000-I1001 I0110             I1001-I1000
 * U1001                                                       U1001                         U1001                         I1000 U1001
 * Prefix shows how this number was created: U - through union and I - through inversion.
 * 
 * For 3 variables we get a giant tree. So we have to unload to disk and 
 * just safe connections between nodes of the tree. Unfortunatly, it takes
 * more than 60 GB and amount of nodes more than 2 billions. So, the algorithm
 * needs futher optimizations.
 */
#include "helper.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cinttypes>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
using namespace std;

template<int VariableCount>
class BinaryFuncTree {
private:

    /*
    * Tree node
    */
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

    /*
    * Try inversion of value. If it gives new function, we will add it to stack
    * and return true. Othewise return false;
    */
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

    /*
    * Try union of left and right. If it gives new function, we will add it to stack
    * and return true. Othewise return false;
    */
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

    /*
    * Try union of each with each functions
    */
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

    /*
    * After an inversion we can just try unions with the inverted value, as we have already tried
    * all other unions in previous steps
    */
    void _union_last(MyStack<iterator, VariableCount>& stack,
                     const typename MyStack<iterator, VariableCount>::iterator& unionBegin) {
        auto& val = *stack.last();
        auto end = stack.last();
        for (auto it = unionBegin; it != end; ++it) {
            _tryUnion(stack, (*it)->func, val->func);
        }
    }

    /*
    * Creating binary array of different objects to save it to disk
    */
    template<class T>
    size_t _set_to_buf(uint8_t* data, size_t size, const T& obj) {
        if (size < sizeof(obj)) {
            return 0;
        } else {
            memcpy_s(data, size, &obj, sizeof(obj));
            return sizeof(obj);
        }
    }
    template<class T, class ...Args>
    size_t _set_to_buf(uint8_t* data, size_t size, const T& obj, const Args&... args) {
        size_t len = _set_to_buf(data, size, obj);
        len += _set_to_buf(data + len, size - len, args...);
        return len;
    }

    /*
    * We assume that childs are already saved. So we can just save parent 
    * and free childs as they are already saved.
    */
    void _save_node_data_and_free_childs(iterator node) {
        //cout << "delete " << node->id << endl;
        uint8_t buf[3000];
        int id = node->id;
        int pid = node->parent ? node->parent->id : -1;


        int len = _set_to_buf(buf, sizeof(buf), "\xBE\xAF\x00\x00", node->id, int(node->func), pid, int(node->origin));

        for (auto& it : node->childs) {
            //cout << " " << it->id;
            len += _set_to_buf(buf + len, sizeof(buf) - len, it->id);
            functionPool.free(it);
        }

        os.write(buf, len);
    }
    /*
    * The main algorithm. It tries inversion and if it succeeds then
    * it tries all unions with the inverted value. And repeat. It 
    * uses functionStack to rememeber all the created functions and
    * uses depthStack to traverse the tree using DFS
    */
    void _generateBinFuncTree(MyStack<iterator, VariableCount>& functionStack) {
        std::list<typename MyStack<iterator, VariableCount>::iterator> depthStack;
        depthStack.push_back(functionStack.begin());
        depthStack.push_back(functionStack.begin());
        auto cur = &depthStack.back();

        unsigned int counter = 0;
        while (true) {
            counter++;
            if (counter % 10000000 == 0) {
                cout << '@' << counter_all << " " << counter_inv << " " << counter_inv / float(counter_all) << endl;
            }
            if (*cur == functionStack.end()) {
                depthStack.pop_back();

                auto start_to_delete = depthStack.back();
                if (free_asap) {
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
    /*
    * If this variable is true, then nodes of the tree are freed as soon as possible 
    * to get enough memory space for all the calculations. 
    * If it is false, then after the algorithm is finished, the tree will be located 
    * in RAM and can be used by other functions like printBinFuncTree
    */
    bool free_asap;
    BinaryFuncTree() : os(cout, 10000), free_asap(false) {}
    BinaryFuncTree(ostream& os, int buffer_size = 10000) : os(os, buffer_size), free_asap(true) {}
    iterator tree_root = iterator();

    /*
    * Starting the algorithm. Sets base functions like 0, 1, x1, ..., xn,
    * unions them all and calls _generateBinFuncTree
    */
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
        if (free_asap) {
            auto end = stack.begin();
            --end;
            for (auto it = stack.last(); it != end; --it) {
                _save_node_data_and_free_childs(*it);
            }
        }
        tree_root = root;
        os.fflush();
    }

    /*
    * Prints fancy tree if free_asap is false
    */
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
    
    std::ios::sync_with_stdio(false); 
    cin.tie(NULL);
    ofstream fout("log.bin", std::ios_base::out | ios::binary);
    BinaryFuncTree<2> bft;
    bft.generateBinFuncTree();
    fout.close();
    bft.printBinFuncTree();
    return 0;
}