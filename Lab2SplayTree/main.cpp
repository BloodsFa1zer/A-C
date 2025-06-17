using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <stdexcept>


struct Rational {
    long long num{0}, den{1};

    Rational(long long n = 0, long long d = 1) {
        if (d == 0) throw invalid_argument("denominator = 0");
        if (d < 0) { n = -n;  d = -d; }
        long long g = gcd(n, d);
        num = n / g;   den = d / g;
    }
    bool operator<(const Rational& o)  const { return num * o.den <  o.num * den; }
    bool operator<=(const Rational& o) const { return !(o < *this); }
    bool operator==(const Rational& o) const { return num == o.num && den == o.den; }

    string toString() const { return to_string(num) + '/' + to_string(den); }
    friend ostream& operator<<(ostream& os, const Rational& r) { return os << r.toString(); }
};


template <typename T> class BinominalHeap;

template <typename T>
class Node {
public:
    Node *parent{}, *sibling{}, *child{};
    T  value{};
    int degree{0};

    Node()  = default;
    explicit Node(const T& v) : value(v) {}

    void printPretty(const string& prefix, bool isTail) const {
        cout << prefix << (isTail ? "└── " : "├── ") << value << '\n';

        vector<Node*> kids;
        for (auto* c = child; c; c = c->sibling) kids.push_back(c);

        for (size_t i = 0; i < kids.size(); ++i)
            kids[i]->printPretty(prefix + (isTail ? "    " : "│   "), i + 1 == kids.size());
    }

    void link(Node* other) {
        parent = other;
        sibling = other->child;
        other->child = this;
        ++other->degree;
    }
    friend BinominalHeap<T>;
};

template <typename T>
class BinominalHeap {
    Node<T>* head = nullptr;
public:
    BinominalHeap() = default;
    explicit BinominalHeap(const T& v) { head = new Node<T>(v); }

    /* -------------------------------------------------- */
    /*                    HELPERS                         */
    /* -------------------------------------------------- */
    void printPretty() const {
        if (!head) { cout << "(empty)\n"; return; }
        for (auto* h = head; h; h = h->sibling)
            h->printPretty("", h->sibling == nullptr);
    }

    Node<T>* minNode() const {
        Node<T>* mn = head;
        for (auto* cur = head; cur; cur = cur->sibling)
            if (cur->value < mn->value) mn = cur;
        return mn;
    }
    Node<T>* prevMinNode() const {
        Node<T>* prev = nullptr, *bestPrev = nullptr, *mn = head;
        for (auto* cur = head; cur; prev = cur, cur = cur->sibling)
            if (cur->value < mn->value) { mn = cur; bestPrev = prev; }
        return bestPrev;
    }

    void merge(BinominalHeap* a, BinominalHeap* b) {
        if (!a->head) { head = b->head; return; }
        if (!b->head) { head = a->head; return; }

        Node<T>* aC = a->head;
        Node<T>* bC = b->head;

        head = (aC->degree <= bC->degree) ? aC : bC;
        if (head == aC) aC = aC->sibling;
        else             bC = bC->sibling;

        Node<T>* tail = head;
        while (aC && bC) {
            if (aC->degree < bC->degree) { tail->sibling = aC; aC = aC->sibling; }
            else                          { tail->sibling = bC; bC = bC->sibling; }
            tail = tail->sibling;
        }
        tail->sibling = aC ? aC : bC;
    }

    void unionHeaps(BinominalHeap* a, BinominalHeap* b) {
        merge(a, b);
        if (!head) return;

        Node<T>* prev = nullptr;
        Node<T>* cur  = head;
        Node<T>* next = cur->sibling;

        while (next) {
            bool needLink = (cur->degree == next->degree) &&
                            ( !(next->sibling && next->sibling->degree == cur->degree) );

            if (!needLink)
            { prev = cur; cur = next; }
            else if (cur->value <= next->value) {
                cur->sibling = next->sibling;
                next->link(cur);
            } else {
                if (!prev) head = next;
                else prev->sibling = next;
                cur->link(next);
                cur = next;
            }
            next = cur->sibling;
        }
    }

    void insert(const T& v) {
        BinominalHeap tmp(v);
        unionHeaps(this, &tmp);
    }

    void extractMin() {
        if (!head) return;
        Node<T>* mnPrev = prevMinNode();
        Node<T>* mn = (mnPrev ? mnPrev->sibling : head);

        if (mnPrev) mnPrev->sibling = mn->sibling;
        else        head = mn->sibling;

        Node<T>* revKids = nullptr;
        for (auto* ch = mn->child; ch; ) {
            auto* nxt = ch->sibling;
            ch->parent = nullptr;
            ch->sibling = revKids;
            revKids = ch;
            ch = nxt;
        }

        BinominalHeap tmp;
        tmp.head = revKids;
        unionHeaps(this, &tmp);
        delete mn;
    }
};

/* ------------------------------------------------------ */
/*                        DEMO                            */
/* ------------------------------------------------------ */
int main() {
    cout << "Creating Heap A:\n";
    BinominalHeap<Rational> A;
    A.insert({3,4});
    A.insert({2,4});
    A.insert({7,3});
    A.insert({5,6});
    A.insert({-1,5});
    A.printPretty();

    cout << "\nCreating Heap B:\n";
    BinominalHeap<Rational> B;
    B.insert({10,7});
    B.insert({-22,7});
    B.insert({123,100});
    B.insert({9,10});
    B.printPretty();

    cout << "\nUnion of A and B => Heap C :\n";
    BinominalHeap<Rational> C;
    C.unionHeaps(&A, &B);
    C.printPretty();

    cout << "\nMinimum element in C: " << C.minNode()->value << "\n\n";

    cout << "After extractMin():\n";
    C.extractMin();
    C.printPretty();

    return 0;
}
