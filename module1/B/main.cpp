///////////////////////////////////////////////////////////////////////////
// Problem: 1B.
//
//  Строка называется палиндромом, если она одинаково читается как
//  слева направо, так и справа налево.
//  Например, «abba» – палиндром, а «roman» – нет.
//
//  Для заданной строки s длины n (1 ≤ n ≤ 10^5) требуется подсчитать
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
#include <cassert>

template<bool is_even_mode>
size_t calcPalindroms(const std::string & data) {
    bool is_uneven_mode = !is_even_mode;

    ssize_t n = data.length();
    std::vector<ssize_t> n_of_palindrom(n);

    ssize_t l = 0, r = -1;
    ssize_t result = 0;

    for (ssize_t i = 0; i < n; ++i) {
        ssize_t cur_palindrom_len = is_uneven_mode;
        //if (r >= 0 && i <= r){ // it is necessary in comparison between int and size_t
        if (i <= r){
            cur_palindrom_len = std::min(n_of_palindrom[l + r - i + is_even_mode], r - i + 1);
        }
        while (i + cur_palindrom_len < n && i - cur_palindrom_len - is_even_mode >= 0 &&
               data[i + cur_palindrom_len] == data[i - cur_palindrom_len - is_even_mode]) {
            ++cur_palindrom_len;
        }

        n_of_palindrom[i] = cur_palindrom_len - is_uneven_mode;
        result += n_of_palindrom[i];

        if (i + cur_palindrom_len - 1 > r) {
            l = i - cur_palindrom_len + is_uneven_mode;
            r = i + cur_palindrom_len - 1;
        }
    }

    return result;
}

enum Even{
    uneven = 0,
    even = 1
};

int main() {
    std::string input;
    std::cin >> input;

    std::cout << calcPalindroms<uneven>(input) + calcPalindroms<even>(input);

    return 0;
}
