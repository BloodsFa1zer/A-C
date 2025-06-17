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

    //── erase / deleteKey (видалення довільного елемента)
    bool erase(const T& key) {
        if (!contains(key))               // contains() вже зробить splay;
            return false;                 // якщо key нема — нічого не міняємо

        Node* toDelete = root_;           // key зараз у корені
        Node* L = root_->left;
        Node* R = root_->right;
        if (L) L->parent = nullptr;
        if (R) R->parent = nullptr;
        delete toDelete;

        if (!L) {                         // лівого піддерева нема → коренем стає R
            root_ = R;
        } else {
            // 1) знайти найбільший вузол у L
            Node* maxL = L;
            while (maxL->right) maxL = maxL->right;
            // 2) підняти його у корінь (зліва усе ≤ maxL)
            root_ = L;
            splay(maxL);                  // тепер maxL - новий root
            // 3) просте приклеювання R праворуч
            maxL->right = R;
            if (R) R->parent = maxL;
        }
        return true;                      // key успішно видалено
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
        //──────── Rational demo ───────────────────────────────────────
        vector<Rational> vec = {
            {3,10},{1,2},{5,6},{7,8},
            {2,3},{9,10},{11,12},{13,14}
        };
        SplayTree<Rational> ratTree;
        for (auto& r : vec) ratTree.insert(r);

        cout << "NEW SPLAY TREE (Rational)\n";
        ratTree.print();

        //-- erase()
        cout << "ERASE 7/8\n";
        ratTree.erase(Rational{7,8});
        ratTree.print();

        cout << "\n──────────────────────────────────────────\n\n";

        //──────── int demo ────────────────────────────────────────────
        SplayTree<int> intTree;
        for (int i = 0; i < 10; ++i) intTree.insert(i);

        cout << "NEW SPLAY TREE (int)\n";
        intTree.print();

        //-- erase existing key
        cout << "ERASE 5\n";
        intTree.erase(5);
        intTree.print();

        //-- erase non-existing key (expect false)
        cout << "ERASE 42 (not present) → "
                  << boolalpha << intTree.erase(42) << "\n";
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
