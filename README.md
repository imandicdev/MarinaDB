# MarinaDB

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
