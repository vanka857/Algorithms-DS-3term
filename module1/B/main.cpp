///////////////////////////////////////////////////////////////////////////
// Problem: 1B.
//
//  Строка называется палиндромом, если она одинаково читается как
//  слева направо, так и справа налево.
//  Например, «abba» – палиндром, а «roman» – нет.
//
//  Для заданной строки s длины n (1 ≤ n ≤ 105) требуется подсчитать
//  число пар (i, j), 1 ≤ i < j ≤ n, таких что подстрока s[i..j]
//  является палиндромом.
//
// Формат ввода:
//  Входной файл содержит одну строку s длины n, состоящюю из маленьких
//  латинских букв.
//
// Формат вывода:
//  В выходной файл выведите искомое число подстрок-палиндромов.
///////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>


int64_t calcPalindroms(const std::string & data) {
    int64_t n = data.length();

    std::vector<int64_t> d1(n);

    int64_t l = 0, r = -1;

    for (int64_t i = 0; i < n; ++i) {
        int64_t k = 1;
        if (i <= r) {
            k = std::min(d1[l + r - i], r - i + 1);
        }
        while (i + k < n && i - k >= 0 && data[i + k] == data[i - k]) {
            ++k;
        }

        d1[i] = k - 1;
        
        if (i + k - 1 > r) {
            l = i - k + 1;
            r = i + k - 1;
        }
    }

    std::vector<int64_t> d2(n);

    l = 0, r = -1;

    for (int64_t i = 0; i < n; ++i) {
        int64_t k = 0;
        if (i <= r){
            k = std::min(d2[l+r-i+1], r-i+1);
        }
        while (i + k < n && i - k - 1 >= 0 && data[i + k] == data[i - k - 1]) {
            ++k;
        }
        
        d2[i] = k;

        if (i + k - 1 > r) {
            l = i - k;
            r = i + k - 1;
        }
    }

    int64_t result = 0;

    for (auto i : d1) {
        result += i;
    }
    for (auto i : d2) {
        result += i;
    }

    return result;
}

int main() {
    std::string input;

    std::cin >> input;

    std::cout << calcPalindroms(input);

    return 0;
}
