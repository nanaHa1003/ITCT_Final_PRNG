#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <array>
#include <cstdint>
#include <random>
#include <algorithm>

// This function will calculate initial random sequence with given parameters
double init_seq_generator(double mu, double x0, int it, int seed, std::vector<uint8_t> &bits) {
    // Initial logistic chaos map iteration
    double xn = x0;
    for (int i = 0; i < it; ++i) {
        xn = mu * xn * (1.0 - xn);
    }

    // Extract digits
    std::array<uint8_t, 15> digits;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << xn;
    for (int i = 0; i < 15; ++i) {
        digits[i] = ss.str()[i + 2] - '0';
    }

    // Prepare minimal PRNG (LCG) for choosing digits
    std::minstd_rand engine(seed);

    // Construct bit sequences
    std::array<uint8_t, 15> index{{ 0,  1,  2,  3 , 4,
                                    5,  6,  7,  8,  9,
                                   10, 11, 12, 13, 14 }};
    bits.resize(13);
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

    return xn;
}

double normal_seq_generator(double state, int seed) {
    // Extract decimal digits from previous generated y (state)
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << state;
    std::array<uint8_t, 15> digits;
    for (int i = 0; i < 15; ++i) {
        digits[i] = ss.str()[i + 2] - '0';
    }

    // Shuffle digits
    std::minstd_rand engine(seed);
    std::random_shuffle(digits.begin(), digits.end(), [&](int max) {
        return engine() % max;
    });

    // Form new mu, x and I for next generation
    int I = 0;
    double mu, x;

    I = I + digits[2];
    I = (I << 3) + (I << 1) + digits[6];
    I = (I << 3) + (I << 1) + digits[10];
    I = (I << 3) + (I << 1) + digits[14];

    std::string mu_s(4, '0');
    mu_s[0] = digits[1] + '0';
    mu_s[1] = digits[5] + '0';
    mu_s[2] = digits[9] + '0';
    mu_s[3] = digits[13] + '0';
    mu = static_cast<double>(std::stoi(mu_s));

    std::string x_s(6, '0');
    x_s[1] = '.';
    x_s[2] = '0' + digits[0];
    x_s[3] = '0' + digits[4];
    x_s[4] = '0' + digits[8];
    x_s[5] = '0' + digits[12];
    x = std::stod(x_s);

    // Refine generated parameters to meet to chaotic condiction
    mu += 3.5699;
    while (mu > 4.0) {
        mu = std::fmod(mu, 4.0) + 3.5699;
    }

    if (I < 1000) {
        I += 1000;
    }

    // Generate next random number with logistic chaotic system
    for (int i = 0; i < I; ++i) {
        x = mu * x * (1.0 - x);
    }

    return x;
}

int main() {
    std::vector<uint8_t> bits;
    double y1 = init_seq_generator(3.8, 0.3, 1024, 42, bits);

    double y = y1;
    for (int i = 0; i < 100; ++i) {
        std::cout << y << std::endl;
        y = normal_seq_generator(y, 42);
    }

    return 0;
}
