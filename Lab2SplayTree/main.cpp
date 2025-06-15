using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <functional>
#include <stdexcept>

//==========================================================================
//  RATIONAL
//==========================================================================
struct Rational {
    long long num{0}, den{1};
    Rational(long long n = 0, long long d = 1) {
        if (d == 0) throw invalid_argument("denominator = 0");
        if (d < 0) { n = -n; d = -d; }
        long long g = gcd(n, d);
        num = n / g; den = d / g;
    }
    bool operator<(const Rational& o) const { return num * o.den < o.num * den; }
    bool operator==(const Rational& o) const { return num == o.num && den == o.den; }
    string toString() const { return to_string(num) + '/' + to_string(den); }
    friend ostream& operator<<(ostream& os, const Rational& r) { return os << r.toString(); }
};

//==========================================================================
//  SPLAY TREE
//==========================================================================
template <typename T, typename Compare = less<T>>
class SplayTree {
    struct Node {
        T     key;
        Node *left{nullptr}, *right{nullptr}, *parent{nullptr};
        explicit Node(const T& k) : key(k) {}
    };

    Node*   root_ = nullptr;
    Compare cmp_  = Compare{};

    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent)       root_ = y;
        else if (x == x->parent->left) x->parent->left  = y;
        else                           x->parent->right = y;
        y->left = x; x->parent = y;
    }
    void rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent)       root_ = y;
        else if (x == x->parent->left) x->parent->left  = y;
        else                           x->parent->right = y;
        y->right = x; x->parent = y;
    }

    void splay(Node* x) {
        if (!x) return;
        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;
            if (!g) {                           // Zig
                if (x == p->left)  rotateRight(p);
                else               rotateLeft(p);
            } else if ((x == p->left) == (p == g->left)) { // Zig-Zig
                if (x == p->left)  { rotateRight(g); rotateRight(p); }
                else               { rotateLeft(g);  rotateLeft(p);  }
            } else {                              // Zig-Zag
                if (x == p->left)  { rotateRight(p); rotateLeft(g); }
                else               { rotateLeft(p);  rotateRight(g);}
            }
        }
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left); destroy(n->right); delete n;
    }

    static void printRec(const Node* n, const string& indent, bool right, ostream& os) {
        if (!n) return;
        printRec(n->right, indent + (right ? "        " : "│       "), true, os);
        os << indent;
        if (right) os << "└───── ";
        else       os << "┌───── ";
        os << n->key << '\n';
        printRec(n->left,  indent + (right ? "        " : "│       "), false, os);
    }

public:
    SplayTree() = default;
    ~SplayTree() { destroy(root_); }
    SplayTree(const SplayTree&) = delete;
    SplayTree& operator=(const SplayTree&) = delete;

    //── insert (no duplicates)
    void insert(const T& key) {
        if (!root_) { root_ = new Node(key); return; }
        Node* cur = root_;
        Node* parent = nullptr;
        bool leftChild = false;
        while (cur) {
            parent = cur;
            if (cmp_(key, cur->key)) { cur = cur->left;  leftChild = true;  }
            else if (cmp_(cur->key, key)) { cur = cur->right; leftChild = false; }
            else { splay(cur); return; }                    // duplicate → splay & ignore
        }
        Node* n = new Node(key);
        n->parent = parent;
        if (leftChild) parent->left = n; else parent->right = n;
        splay(n);
    }

    //── find (returns true/false, splays found/last accessed)
    bool contains(const T& key) {
        Node* cur = root_;
        Node* last = nullptr;
        while (cur) {
            last = cur;
            if (cmp_(key, cur->key)) cur = cur->left;
            else if (cmp_(cur->key, key)) cur = cur->right;
            else { splay(cur); return true; }
        }
        if (last) splay(last);
        return false;
    }

    //── extract minimum (splays minimum to root, then removes)
    T extractMin() {
        if (!root_) throw runtime_error("tree empty");
        Node* cur = root_;
        while (cur->left) cur = cur->left;
        splay(cur);                                 // now cur = root
        T res = cur->key;
        Node* r = cur->right;
        if (r) r->parent = nullptr;
        delete cur;
        root_ = r;
        return res;
    }

    bool empty() const noexcept { return root_ == nullptr; }

    //── visualisation
    void print(ostream& os = cout) const {
        if (!root_) { os << "(empty)\n"; return; }
        printRec(root_, "", true, os);
    }
};

//==========================================================================
//  MAIN
//==========================================================================
int main() {
    try {
        //──── Rational demo ──────────────────────────────────────────
        vector<Rational> vec = { {3,10},{1,2},{5,6},
            {7,8},{2,3},{9,10},{11,12},{13,14} };
        SplayTree<Rational> ratTree;
        for (auto& r : vec) ratTree.insert(r);

        cout << "NEW SPLAY TREE (Rational)\n";
        ratTree.print();

        cout << "EXTRACT MIN\n";
        ratTree.extractMin();
        ratTree.print();

        cout << "\n──────────────────────────────────────────────────────────\n\n";

        //──── int demo ───────────────────────────────────────────────
        SplayTree<int> intTree;
        for (int i = 0; i < 10; ++i) intTree.insert(i);

        cout << "NEW SPLAY TREE (int)\n";
        intTree.print();

        cout << "EXTRACT MIN\n";
        intTree.extractMin();
        intTree.print();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
