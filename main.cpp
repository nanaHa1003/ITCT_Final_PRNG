#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <array>
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

    I  = digits[2] * 1000 + digits[6] * 100 + digits[10] * 10 + digits[14];
    mu = digits[1] * 1000 + digits[5] * 100 + digits[9] * 10 + digits[13];
    x  = digits[0] * 0.1 + digits[4] * 0.01 + digits[8] * 0.001 + digits[12] * 0.0001;

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

template <typename Container1, typename Container2>
void from_double_to_bytes(Container1 &nums, Container2 &bytes, int seed) {
    std::array<uint8_t, 15> digits;
    std::minstd_rand engine(seed);
    std::array<uint8_t, 15> index{{ 0,  1,  2,  3 , 4,
                                    5,  6,  7,  8,  9,
                                   10, 11, 12, 13, 14 }};

    bytes.resize(nums.size() * 13);
    for (size_t i = 0; i < nums.size(); ++i) {
        // Extract digits
        extract_decimal_digits(nums[i], digits);

        // Construct bit sequences
        for (int j = 0; j < 13; ++j) {
            // Generate new order of indices
            std::random_shuffle(index.begin(), index.end(), [&](int max) {
                return engine() % max;
            });

            // Convert selected digits to number and mod by 256
            uint64_t y = 0;
            for (int k = 0; k < j + 3; ++k) {
                y = (y << 3) + (y << 1) + digits[index[k]];
            }

            // Save results
            bytes[j + 13 * i] = static_cast<uint8_t>(y % 256ULL);
        }
    }
}

int main(int argc, char **argv) {
    std::cout << "Start initial sequence generaton" << std::endl;

    std::vector<uint8_t> bytes;
    double y1 = init_seq_generator(3.6779, 0.6942, 8459, 42, bytes);

    std::cout << "Initial sequence generation completed" << std::endl;
    for (auto num : bytes) {
        for (int i = 7; i >= 0; --i) {
            std::cout << ((num & (1 << i)) >> i);
        }
    }
    std::cout << std::endl;

    std::vector<double> data(20000);
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

    from_double_to_bytes(data, bytes, 4096);
    std::ofstream bin("seq.bin", std::ios::out | std::ios::binary);
    bin.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    bin.close();

    return 0;
}
