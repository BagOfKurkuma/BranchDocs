#include "Uuid.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace branchdocs::domain {

std::string generateUuid() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dist;

    uint64_t a = dist(gen);
    uint64_t b = dist(gen);
    a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << static_cast<uint32_t>(a >> 32) << '-'
        << std::setw(4) << static_cast<uint16_t>((a >> 16) & 0xFFFF) << '-'
        << std::setw(4) << static_cast<uint16_t>(a & 0xFFFF) << '-'
        << std::setw(4) << static_cast<uint16_t>(b >> 48) << '-'
        << std::setw(12) << (b & 0xFFFFFFFFFFFFULL);
    return oss.str();
}

} // namespace branchdocs::domain
