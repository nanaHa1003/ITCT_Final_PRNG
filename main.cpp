#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>

// This function will calculate initial random sequence with given parameters
std::vector<uint8_t> init_seq_generator(double mu, double x0, int it) {
    // Initial logistic chaos map iteration
    double xn = x0;
    for (int i = 0; i < it; ++i) {
        xn = mu * xn * (1.0 - xn);
    }

    // Extract digits
    std::vector<uint8_t> digits(15);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << xn;
    for (int i = 0; i < 15; ++i) {
        digits[i] = ss.str()[i + 2] - '0';
    }

    // Construct bit sequences
    std::vector<uint8_t> bits(13);
    for (int i = 0; i < bits.size(); ++i) {
        // Use 64 bit int is enough for 15 digits
        uint64_t y = 0;
        for (int j = 0; j < i + 3; ++j) {
            y = (y << 3) + (y << 1) + digits[j];
        }
        bits[i] = static_cast<uint8_t>(y % 256ULL);
        std::cout << int(bits[i]) << std::endl;
    }

    return bits;
}



int main() {
    auto bits = init_seq_generator(3.8, 0.3, 1024);
    return 0;
}
