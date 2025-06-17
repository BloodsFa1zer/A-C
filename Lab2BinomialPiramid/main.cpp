using namespace std;

#include <iostream>
#include <string>
#include <vector>

template <typename T>
class BinominalHeap;

template <typename T>
class Node {
public:
    Node *parent, *sibling, *child;
    T      value;
    int    degree;

    Node() : parent(nullptr), sibling(nullptr), child(nullptr), value(), degree(0) {}
    explicit Node(const T& newValue) : Node() { value = newValue; }
    
    void print(int tabs) const {
        for (int i = 0; i < tabs; ++i) cout << '\t';
        cout << value << ":" << degree;
        if (child)  { cout << '\n'; child->print(tabs + 1); }
        if (sibling){ cout << '\n'; sibling->print(tabs);    }
    }

    void printPretty(const string& prefix, bool isTail) const {
        cout << prefix << (isTail ? "└── " : "├── ") << value << ":" << degree << '\n';

        vector<Node*> kids;
        for (auto* ch = child; ch; ch = ch->sibling) kids.push_back(ch);

        for (size_t i = 0; i < kids.size(); ++i) {
            kids[i]->printPretty(prefix + (isTail ? "    " : "│   "),
                                 i + 1 == kids.size());
        }
    }

    void link(Node* other) {           
        parent   = other;
        sibling  = other->child;
        other->child = this;
        ++other->degree;
    }
    friend ostream& operator<<(ostream& out, const Node& n) {
        return out << n.value;
    }
    friend class BinominalHeap<T>;
};

template <typename T>
class BinominalHeap {
    Node<T>* head = nullptr;
public:
    BinominalHeap() = default;
    explicit BinominalHeap(const T& v) { head = new Node<T>(v); }
    ~BinominalHeap() = default;

    /* красивий друк для всієї піраміди */
    void printPretty() const {
        if (!head) { cout << "(empty)\n"; return; }
        for (Node<T>* cur = head; cur; cur = cur->sibling)
            cur->printPretty("", cur->sibling == nullptr);
    }

    void merge(BinominalHeap* a, BinominalHeap* b) {
        if (!a->head) { head = b->head; return; }
        if (!b->head) { head = a->head; return; }

        Node<T>* h;                     // початковий вузол об'єднаної кореневої черги
        Node<T>* aCur = a->head;
        Node<T>* bCur = b->head;

        if (aCur->degree <= bCur->degree) {
            h = aCur;  aCur = aCur->sibling;
        } else {
            h = bCur;  bCur = bCur->sibling;
        }
        head = h;

        Node<T>* tail = h;
        while (aCur && bCur) {
            if (aCur->degree < bCur->degree) {
                tail->sibling = aCur;  aCur = aCur->sibling;
            } else {
                tail->sibling = bCur;  bCur = bCur->sibling;
            }
            tail = tail->sibling;
        }
        tail->sibling = aCur ? aCur : bCur;
    }

    void unionHeaps(BinominalHeap* a, BinominalHeap* b) {
        merge(a, b);
        if (!head) return;

        Node<T>* prev = nullptr;
        Node<T>* cur  = head;
        Node<T>* next = cur->sibling;

        while (next) {
            if (cur->degree != next->degree ||
               (next->sibling && next->sibling->degree == cur->degree)) {
                prev = cur; cur = next;
            }
            else if (cur->value <= next->value) {          // cur стає батьком
                cur->sibling = next->sibling;
                next->link(cur);
            }
            else {                                         // next стає батьком
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

    Node<T>* minNode() const {
        Node<T>* mn = head;
        for (Node<T>* cur = head; cur; cur = cur->sibling)
            if (cur->value < mn->value) mn = cur;
        return mn;
    }

    Node<T>* prevMinNode() const {
        Node<T>* prev = nullptr, *bestPrev = nullptr;
        Node<T>* mn = head;
        for (Node<T>* cur = head; cur; prev = cur, cur = cur->sibling)
            if (cur->value < mn->value) { mn = cur; bestPrev = prev; }
        return bestPrev;
    }

    void extractMin() {
        if (!head) return;
        Node<T>* mnPrev = prevMinNode();
        Node<T>* mn     = (mnPrev ? mnPrev->sibling : head);
        
        if (mnPrev) mnPrev->sibling = mn->sibling;
        else        head            = mn->sibling;

        Node<T>* revKids = nullptr;
        for (Node<T>* ch = mn->child; ch; ) {
            Node<T>* next = ch->sibling;
            ch->parent = nullptr;
            ch->sibling = revKids;
            revKids = ch;
            ch = next;
        }

        BinominalHeap tmp;
        tmp.head = revKids;
        unionHeaps(this, &tmp);
        delete mn;
    }
};

/* ------------------------------------------------------ */
/*                    DEMO-ПРОГРАМА                       */
/* ------------------------------------------------------ */
int main() {
    BinominalHeap<string> h1;
    h1.insert("orange");
    h1.insert("apple");
    h1.insert("grape");

    cout << "Heap 1:\n";
    h1.printPretty();

    BinominalHeap<string> h2;
    h2.insert("pear");
    h2.insert("banana");
    h2.insert("cherry");

    cout << "\nHeap 2:\n";
    h2.printPretty();

    BinominalHeap<string> merged;
    merged.unionHeaps(&h1, &h2);

    cout << "\nAfter union:\n";
    merged.printPretty();

    cout << "\nMin element: " << merged.minNode()->value << "\n\n";
    merged.extractMin();

    cout << "After extractMin():\n";
    merged.printPretty();
}
