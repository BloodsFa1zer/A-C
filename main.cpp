using namespace std;

#include <iostream>
#include <vector>
#include <numeric>
#include <functional>
#include <stdexcept>
#include <string>

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
    string toString() const {
        return to_string(num) + '/' + to_string(den);
    }
    friend ostream& operator<<(ostream& os, const Rational& r) {
        return os << r.toString();
    }
};

//==========================================================================
//  BINOMIAL HEAP
//==========================================================================
template <typename T, typename Compare = less<T>>
class BinomialHeap {
public:
    struct Node {
        T      key;
        int    degree   = 0;
        Node*  parent   = nullptr;
        Node*  child    = nullptr;
        Node*  sibling  = nullptr;
        explicit Node(const T& k) : key(k) {}
    };

private:
    Node*   head_ = nullptr;
    Compare cmp_  = Compare{};

    static Node* mergeRoots(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        Node* head = nullptr; Node** tail = &head;
        while (a && b) {
            if (a->degree <= b->degree) { *tail = a; a = a->sibling; }
            else                         { *tail = b; b = b->sibling; }
            tail = &((*tail)->sibling);
        }
        *tail = (a ? a : b);
        return head;
    }
    static void linkTrees(Node* y, Node* z) {
        y->parent = z; y->sibling = z->child; z->child = y; ++z->degree;
    }
    Node* unite(Node* h1, Node* h2) {
        Node* newHead = mergeRoots(h1, h2);
        if (!newHead) return nullptr;
        Node* prev = nullptr; Node* cur = newHead; Node* next = cur->sibling;
        while (next) {
            if (cur->degree != next->degree ||
                (next->sibling && next->sibling->degree == cur->degree)) {
                prev = cur; cur = next;
            } else {
                if (cmp_(next->key, cur->key)) {
                    if (prev) prev->sibling = next; else newHead = next;
                    linkTrees(cur, next); cur = next;
                } else {
                    cur->sibling = next->sibling;
                    linkTrees(next, cur);
                }
            }
            next = cur->sibling;
        }
        return newHead;
    }
    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->child); destroy(n->sibling); delete n;
    }

    static void collectChildren(const Node* n, vector<const Node*>& v) {
        for (const Node* c = n->child; c; c = c->sibling) v.push_back(c);
    }
    static void printSub(const Node* n,
                         const string& prefix,
                         bool isLast,
                         ostream& os) {
        if (!n) return;
        os << prefix << (isLast ? "└─" : "├─") << n->key << '\n';
        vector<const Node*> kids;
        collectChildren(n, kids);
        for (size_t i = 0; i < kids.size(); ++i) {
            bool lastChild = (i + 1 == kids.size());
            printSub(kids[i],
                     prefix + (isLast ? "   " : "│  "),
                     lastChild,
                     os);
        }
    }

public:
    BinomialHeap() = default;
    ~BinomialHeap() { destroy(head_); }
    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    bool empty() const noexcept { return head_ == nullptr; }

    void insert(const T& v) {
        auto* single = new Node(v);
        head_ = unite(head_, single);
    }
    T extractMin() {
        if (empty()) throw runtime_error("heap empty");
        Node *prevMin=nullptr,*min=rootHead();
        for(Node *prev=nullptr,*cur=head_;cur;prev=cur,cur=cur->sibling)
            if(cmp_(cur->key,min->key)){min=cur;prevMin=prev;}
        if(prevMin) prevMin->sibling=min->sibling; else head_=min->sibling;
        Node* rev=nullptr;
        for(Node* c=min->child;c;){Node* nxt=c->sibling;c->sibling=rev;c->parent=nullptr;rev=c;c=nxt;}
        head_=unite(head_,rev);
        T res=min->key; delete min; return res;
    }

    //── Visualisation ────────────────────────────────────────────────
    void print(ostream& os = cout) const {
        if (empty()) { os << "(empty)\n"; return; }
        size_t idx = 0;
        for (const Node* r = head_; r; r = r->sibling, ++idx) {
            os << "Binomial tree #" << idx << " (degree " << r->degree << ")\n";
            printSub(r, "", true, os); os << '\n';
        }
    }

private:
    Node* rootHead() const { return head_; }
};

//==========================================================================
//  DEMONSTRATION
//==========================================================================
int main() {
    try {
        //── Rational demo ────────────────────────────────────────────
        vector<Rational> fracs = { {3,10},{1,2},{5,6},{7,8},
                                        {2,3},{9,10},{11,12},{13,14} };
        BinomialHeap<Rational> ratHeap;
        for (const auto& r : fracs) ratHeap.insert(r);

        cout << "NEW TREE (Rational)\n";
        ratHeap.print();

        cout << "EXTRACT MIN\n";
        ratHeap.extractMin();
        ratHeap.print();

        cout << "\n──────────────────────────────────────────────────────────\n\n";

        //── Integer demo ─────────────────────────────────────────────
        BinomialHeap<int> intHeap;
        for (int i = 0; i < 10; ++i) intHeap.insert(i);

        cout << "NEW TREE (int)\n";
        intHeap.print();

        cout << "EXTRACT MIN\n";
        intHeap.extractMin();
        intHeap.print();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
