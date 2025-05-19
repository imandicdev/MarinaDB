# MarinaDB

**Relational Database Engine in Modern C++ (Educational, Fast, and Indexed)**

---

## Overview

MarinaDB is a lightweight, modular relational database engine written from scratch in C++. It demonstrates crucial database internals: table schemas, persistent storage, a command-line interface, and most importantly, efficient [B+ tree](https://en.wikipedia.org/wiki/B%2B_tree) indexing for rapid primary key lookups. Designed for clarity, performance, and extensibility, MarinaDB is a great codebase for learning, for interviews, or as a foundation for further research or prototyping.

## Key Features

- **Relational table design**: custom schema definitions, multiple column types (int, float, string)
- **Indexed access**: built-in B+ tree for O(log N) primary key queries
- **Command-line interface (CLI)**: human-friendly prompt with support for creation, loading, insert, select, and schema management
- **Persistent storage**: databases can be saved to/loaded from disk files
- **Extensible and modern**: clean C++17, modular layout, simple code to review and extend
- **Comprehensive benchmarks**: demonstrates the dramatic speedup from using an index

## Project Goals

- **Education**: Understand the architecture and inner workings of a database engine
- **Efficiency**: Achieve fast indexed queries with proven data structures
- **Extensibility**: Easy-to-read codebase, suitable for adding features (types, query language, etc)
- **Minimal dependencies**: Just C++ and standard library, portable to any major platform

## Implementation Highlights

- **B+ Tree Index** for primary keys: enables fast, scalable lookup and insertion.
- **Schema and Table Abstractions**: supports varying types, handles command parsing and schema enforcement.
- **Command Dispatcher**: modular routing from CLI words to operations.
- **Serialization/Deserialization**: save/load to filesystem using simple formats.
- **Testing and Benchmarking Suite**: compare index vs scan and demonstrate performance at scale.

## Getting Started

### Dependencies

- C++17 compatible compiler (tested with MSVC and GCC)
- [CMake](https://cmake.org/) (for cross-platform build)
- (Optionally) CLion, Visual Studio, or your preferred IDE

### Build

```sh
git clone https://github.com/YOUR_USER/MarinaDB.git
cd MarinaDB
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run CLI

```sh
cd Release
./MarinaDB.exe
```

### Run Benchmark

```sh
./benchmark_table_index.exe
```

## Example CLI Session

```
MarinaDB CLI v0.2. Type 'help' for commands.
(C) 2024-2025 Ilija Mandic. All rights reserved.
marina> help
Supported commands:
  create <file>                             Create a new (empty) database
  load <file>                               Load existing database
  create_table <table> <col>:<type> ...     Create table/schema (types: int, float, string)
  insert <table> <col>=<val> ...            Insert record into table
  select <table>                            Display all records from table
  select <table> where <column>=<value>     Find and print a record by key (fast if indexed, else linear)
  help                                      Show this message
  exit                                      Quit MarinaDB CLI

marina> create testdb.marina
Empty database created and saved to testdb.marina
marina> create_table users id:int name:string
Table 'users' created.
marina> insert users id=1 name=Alice
Inserted record into 'users'.
marina> select users
id      name
1       Alice
marina> exit
Goodbye!
```

## Example Benchmark Output

```
Inserted 1000000 records (BPlusTree index enabled: true)
Probe key (random): 999999
Indexed (BPlusTree) lookup time: 71.475 ms. Result: row_999999
Linear scan lookup time:     179.050 ms. Result: row_999999

INDEX SPEEDUP: 2.471x faster
This output demonstrates the efficiency of BPlusTree-based indexing.
Bulk access (sequential retrieval of all records) time: 56.905 ms. Summed id: 499999500000
```

## Roadmap & Contribution

- Planned: Multi-column support, secondary indexes, transaction/logging, SQL subset
- Community feedback is welcome!  
- To contribute: fork, branch, and PR. For bugfixes, clearly describe the scenario and include a test.

## Author & Contact

- Author: **Ilija Mandic**
- [Your GitHub profile/link]
- For questions, suggestions, or invitations (including internship/interview context!), open an issue or contact via GitHub.

---

*(This project is for educational, demo, and light experimental use. It's not intended for production deployment, but it's a great start for serious DB internals work!)*

A simple, modern C++20 database engine for learning and experimentation.  
Implements basic table, schema, and record management, with planned support for file persistence and indexing.

**Features:**
- Modern C++20 codebase (RAII, smart pointers, STL algorithms)
- Basic schema and table management
- Designed for cross-platform compatibility (Windows & Linux)
- Clear, incremental commit history for educational purposes

**Build Instructions:**
```bash
git clone https://github.com/imandicdev/MarinaDB.git
cd MarinaDB
mkdir build && cd build
cmake ..
cmake --build .
./MarinaDB
