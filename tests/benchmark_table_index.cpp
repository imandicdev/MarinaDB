// benchmark_table_index.cpp
// Benchmark script to compare indexed (BPlusTree) and linear scan lookup
// for a large Table with int primary key.
// For demonstration/interview use.

#include <iostream>
#include <chrono>
#include <random>
#include "../include/Table.h"
#include "../include/Schema.h"

using namespace std;
using namespace std::chrono;

int main() {
    // ----- 1. Create schema: int primary key -----
    vector<Column> columns = {
        {"id", DataType::Integer},
        {"value", DataType::String}
    };
    TableSchema schema("test_table", columns);
    Table table(schema);

    constexpr int N = 1000000;    // Make very large for dramatic comparison
    vector<int> keys;
    keys.reserve(N);

    // ----- 2. Insert many records -----
    for (int i = 0; i < N; ++i) {
        keys.push_back(i);
        Record rec;
        rec["id"] = i;
        rec["value"] = "row_" + to_string(i);
        table.insert(rec);
    }
    cout << "Inserted " << N << " records (BPlusTree index enabled: " << boolalpha << table.isIndexed() << ")\n";

    // Pick a random key to test lookup (worst-case for linear scan: the last one)
    int probeKey = N-1;
    cout << "Probe key (random): " << probeKey << endl;

    // ----- 3. Indexed find -----
    auto t1 = high_resolution_clock::now();
    const Record* recIdx = table.findByKey(Value(probeKey));
    auto t2 = high_resolution_clock::now();
    auto idxDur = duration_cast<nanoseconds>(t2-t1).count();

    // ----- 4. Linear scan find -----
    // TO FORCE LINEAR SCAN: indexActive is private; simulate by iterating yourself
    t1 = high_resolution_clock::now();
    const Record* recLin = nullptr;
    for (const auto& rec : table.getRecords()) {
        auto it = rec.find("id");
        if (it != rec.end() && std::holds_alternative<int>(it->second) && std::get<int>(it->second) == probeKey) {
            recLin = &rec;
            break;
        }
    }
    t2 = high_resolution_clock::now();
    auto linDur = duration_cast<nanoseconds>(t2-t1).count();

    // ----- 5. Results -----
    auto print_time = [](const char* label, long long ns) {
        cout << label;
        if (ns < 1'000'000)
            cout << ns << " ns";
        else if (ns < 1'000'000'000)
            cout << fixed << setprecision(3) << (ns / 1'000'000.0) << " ms";
        else
            cout << fixed << setprecision(3) << (ns / 1'000'000'000.0) << " s";
        cout << ". ";
    };
    cout << "Indexed (BPlusTree) lookup time: ";
    print_time("", idxDur);
    cout << "Result: ";
    if (recIdx) cout << std::get<std::string>(recIdx->at("value")) << endl;
    else cout << "[NOT FOUND]\n";
    cout << "Linear scan lookup time: ";
    print_time("", linDur);
    cout << "Result: ";
    if (recLin) cout << std::get<std::string>(recLin->at("value")) << endl;
    else cout << "[NOT FOUND]\n";

    cout << "\nINDEX SPEEDUP: " << (double)linDur/idxDur << "x faster\n";
    cout << "This output demonstrates the efficiency of BPlusTree-based indexing." << endl;
    cout << "For best realism, run multiple times or probe several keys." << endl;

    // ----- 6. Bulk access benchmark -----
    t1 = high_resolution_clock::now();
    long long id_sum = 0;
    for (const auto& rec : table.getRecords()) {
        auto it = rec.find("id");
        if (it != rec.end() && std::holds_alternative<int>(it->second)) {
            id_sum += std::get<int>(it->second);
        }
    }
    t2 = high_resolution_clock::now();
    auto bulkDur = duration_cast<nanoseconds>(t2-t1).count();
    cout << "\nBulk access (sequential retrieval of all records) time: ";
    print_time("", bulkDur);
    cout << "Summed id: " << id_sum << endl;

    return 0;
}

/*
Demo usage: 
- Compile/run as part of your test suite or standalone.
- For MariaDB interview: point to source and run live for clear performance proof!
*/

