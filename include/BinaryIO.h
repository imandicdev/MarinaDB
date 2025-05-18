//
// Created by Ilija Mandic on 4/18/2024.
//

#pragma once
#include <cstdint>
#include <ostream>
#include <istream>
#include <string>

inline void write_uint8(std::ostream& os, uint8_t v)    { os.write(reinterpret_cast<char *>(&v), 1); }
inline void write_uint16(std::ostream& os, uint16_t v)  { os.write(reinterpret_cast<char *>(&v), 2); }
inline void write_uint32(std::ostream& os, uint32_t v)  { os.write(reinterpret_cast<char *>(&v), 4); }
inline void write_string(std::ostream& os, const std::string& str) {
    write_uint16(os, static_cast<std::uint16_t>(str.size()));
    os.write(str.data(), str.size());
}

inline uint8_t read_uint8(std::istream& is)   { uint8_t v;  is.read(reinterpret_cast<char *>(&v), 1); return v; }
inline uint16_t read_uint16(std::istream& is) { uint16_t v; is.read(reinterpret_cast<char *>(&v), 2); return v; }
inline uint32_t read_uint32(std::istream& is) { uint32_t v; is.read(reinterpret_cast<char *>(&v), 4); return v; }
inline std::string read_string(std::istream& is) {
    uint16_t len = read_uint16(is);
    std::string str(len, '\0');
    is.read(&str[0], len);
    return str;
}
