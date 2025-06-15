using namespace std;

#include <iostream>
#include <string>

// ────────────────────────────
// Примітивний динамічний масив
// ────────────────────────────
struct Bucket {
    string* data = nullptr;
    int          sz   = 0;
    int          cap  = 0;

    void push(const string& s) {
        if (sz == cap) {                     // збільшуємо вдвічі
            int newCap = cap ? cap * 2 : 4;
            auto* tmp = new string[newCap];
            for (int i = 0; i < sz; ++i) tmp[i] = data[i];
            delete[] data;
            data = tmp;
            cap  = newCap;
        }
        data[sz++] = s;
    }

    ~Bucket() { delete[] data; }
};

// ────────────────────────────
// Рядок другого рівня
// ────────────────────────────
struct HashRow {
    int          a = 0, b = 0;   // параметри хеш-функції
    int          size = 0;       // m_i = k²
    string* table = nullptr;

    ~HashRow() { delete[] table; }

    void print() const {
        cout << "a=" << a << "  b=" << b << "  size=" << size << '\n';
        for (int i = 0; i < size; ++i)
            if (!table[i].empty()) cout << '[' << i << "]  " << table[i] << '\n';
        cout << "-------------------------\n";
    }
};

// ────────────────────────────
// Перфект-хеш-таблиця для рядків
// ────────────────────────────
class PerfectHashTable {
    const int P = 10000019;            // велике просте
    int       m = 0;                   // кількість бакетів (=n)
    Bucket*   buckets = nullptr;       // 1-й рівень
    HashRow*  rows    = nullptr;       // 2-й рівень

    // Поліноміальний хеш рядка
    static long long strHash(const string& s, int mod) {
        long long h = 0;
        const int BASE = 257;
        for (unsigned char c : s) h = (h * BASE + c) % mod;
        return h;
    }
    // Універсальна функція: ((a·x + b) mod P) mod M
    static int uh(int a, int b, int p, int m, long long x) {
        return int(((a * x + b) % p) % m);
    }

public:
    PerfectHashTable()            { srand(time(nullptr)); }
    PerfectHashTable(const PerfectHashTable&) = delete;
    PerfectHashTable& operator=(const PerfectHashTable&) = delete;
    ~PerfectHashTable()           { delete[] buckets; delete[] rows; }

    // Побудова за масивом рядків (values, n)
    void build(const string* v, int n) {
        m = n;
        buckets = new Bucket[m];

        // — 1-й рівень (фіксовані a1,b1) —
        const int a1 = 31, b1 = 17;
        for (int i = 0; i < n; ++i) {
            int idx = uh(a1, b1, P, m, strHash(v[i], P));
            buckets[idx].push(v[i]);
        }

        // — 2-й рівень (підбір a,b без колізій) —
        rows = new HashRow[m];
        for (int i = 0; i < m; ++i) {
            int k = buckets[i].sz;
            if (!k) continue;

            int mi = k * k;
            rows[i].size  = mi;
            rows[i].table = new string[mi];

            bool ok = false;
            for (int tr = 0; tr < 1000 && !ok; ++tr) {
                int a = rand() % (P - 1) + 1;
                int b = rand() % P;
                bool* used = new bool[mi]();     // 0-ініціалізація

                ok = true;
                for (int j = 0; j < k && ok; ++j) {
                    int idx = uh(a, b, P, mi, strHash(buckets[i].data[j], P));
                    if (used[idx]) ok = false;
                    else used[idx] = true;
                }

                if (ok) {
                    rows[i].a = a; rows[i].b = b;
                    for (int x = 0; x < mi; ++x) rows[i].table[x].clear();
                    for (int j = 0; j < k; ++j) {
                        int idx = uh(a, b, P, mi, strHash(buckets[i].data[j], P));
                        rows[i].table[idx] = buckets[i].data[j];
                    }
                }
                delete[] used;
            }
        }
    }

    // Перевірка наявності рядка
    bool contains(const string& s) const {
        if (m == 0) return false;
        const int a1 = 31, b1 = 17;
        int bkt = uh(a1, b1, P, m, strHash(s, P));
        if (!rows[bkt].size) return false;
        int idx = uh(rows[bkt].a, rows[bkt].b, P, rows[bkt].size, strHash(s, P));
        return rows[bkt].table[idx] == s;
    }

    void print() const {
        for (int i = 0; i < m; ++i)
            if (rows[i].size) { cout << "== bucket " << i << " ==\n"; rows[i].print(); }
    }
};

// ────────────────────────────
// Демонстрація
// ────────────────────────────
int main() {
    const string words[] = {
        "apple", "banana", "grape", "kiwi", "lemon",
        "mango", "orange", "peach", "plum", "watermelon"
    };
    const int N = size(words);

    PerfectHashTable ht;
    ht.build(words, N);

    ht.print();

    cout << boolalpha;
    cout << "contains(\"kiwi\")   = " << ht.contains("kiwi")   << '\n';
    cout << "contains(\"papaya\") = " << ht.contains("papaya") << '\n';
}
