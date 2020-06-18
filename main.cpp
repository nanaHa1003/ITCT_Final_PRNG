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

template <typename Container>
void extract_decimal_digits(double x, Container &digits) {
    for (int i = 0; i < 15; ++i) {
        x = std::fmod(x * 10.0, 10.0);
        digits[i] = int(x);
    }
}

template <typename Iterator>
void robust_random_shuffle(Iterator begin, Iterator end, int seed) {
    std::minstd_rand engine(seed);

    std::vector<uint32_t> indices(end - begin);
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
    }

    // Generate acceptable permutation
    bool acceptable = false;
    while (!acceptable) {
        int count = 0;
        std::random_shuffle(indices.begin(), indices.end(), [&](int max) {
            return engine() % max;        
        });
    
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i] != i) {
                ++count;   
            }
        }

        if (count > 12) {
            acceptable = true;
        }
    }

    // Apply permutation
    for (int i = 0; i < end - begin; ++i) {
        size_t curr = i;
        size_t next = indices[i];
        while (next != i) {
            std::swap(begin[curr], begin[next]);
            indices[curr] = curr;
            curr = next;
            next = indices[next];
        }
        indices[curr] = curr;
    }
}

double init_seq_generator(double mu, double x0, int it, int seed, std::vector<uint8_t> &bits) {
    // Initial logistic chaos map iteration
    double xn = x0;
    for (int i = 0; i < it; ++i) {
        xn = mu * xn * (1.0 - xn);
    }

    // Extract digits
    std::array<uint8_t, 15> digits;
    extract_decimal_digits(xn, digits);

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
    std::array<uint8_t, 15> digits;
    extract_decimal_digits(state, digits);

    // Shuffle digits
    robust_random_shuffle(digits.begin(), digits.end(), seed);

    // Form new mu, x and I for next generation
    int I = 0;
    double mu, x;

    I = digits[2] * 1000 + digits[6] * 100 + digits[10] * 10 + digits[14];

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

    I = (I < 1000) ?(I + 1000) :I;

    // Generate next random number with logistic chaotic system
    for (int i = 0; i < I; ++i) {
        x = mu * x * (1.0 - x);
    }

    return x;
}

int main(int argc, char **argv) {
    std::cout << "Start initial sequence generaton" << std::endl;

    std::vector<uint8_t> bits;
    double y1 = init_seq_generator(3.8, 0.3, 1024, 42, bits);

    std::cout << "Initial sequence generation completed" << std::endl;

    std::vector<double> data(30000);
    data[0] = y1;
    for (size_t i = 1; i < data.size(); ++i) {
        data[i] = normal_seq_generator(data[i - 1], i);
    }

    std::ofstream hist("hist.txt");
    hist << std::fixed << std::setprecision(15);
    for (size_t i = 0; i < data.size(); ++i) {
        hist << data[i] << "\n";
    }
    hist.close();

    std::ofstream bin("seq.bin", std::ios::out | std::ios::binary);
    for (size_t i = 0; i < data.size(); ++i) {
        // Take last 48 bits from double
        char *tmp = reinterpret_cast<char *>(&data[i]) + 2;
        bin.write(tmp, 6);
    }
    bin.close();

    return 0;
}
