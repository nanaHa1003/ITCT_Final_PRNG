#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <random>
#include <algorithm>

// This function will calculate initial random sequence with given parameters
std::vector<uint8_t> init_seq_generator(double mu, double x0, int it, int seed) {
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

    // Prepare minimal PRNG for choosing digits
    std::minstd_rand engine(seed);

    // Construct bit sequences
    std::vector<uint8_t> bits(13);
    std::vector<uint8_t> index({{ 0,   1,  2,  3 , 4,
                                  5,   6,  7,  8,  9,
                                  10, 11, 12, 13, 14 }});
    for (int i = 0; i < bits.size(); ++i) {
        // Generate new order of indices
        std::random_shuffle(index.begin(), index.end(), [&](int max) {
            return engine() % max;
        });

        // Convert selected digits to number and mod by 256
        uint64_t y = 0;
        for (int j = 0; j < i + 3; ++j) {
            y = (y << 3) + (y << 1) + digits[index[j]];
        }

        // Save results
        bits[i] = static_cast<uint8_t>(y % 256ULL);
    }

    return bits;
}

int main() {
    auto bits = init_seq_generator(3.8, 0.3, 1024, 42);
    return 0;
}
