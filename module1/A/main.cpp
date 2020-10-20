///////////////////////////////////////////////////////////////////////////////
// Problem: 1A.
// 
// Найдите все вхождения шаблона в строку. 
// Длина шаблона – p, длина строки – n. 
// Время O(n + p), доп. память – O(p).
// p ≤ 30000, n ≤ 300000. 
//
// Вариант 1. С помощью префикс-функции
//
// Формат ввода
// Шаблон, символ перевода строки, строка.
//
// Формат вывода
// Позиции вхождения шаблона в строке.

// Пример:

// Ввод:
// a
// aaa

// Вывод:
// 0 1 2

#include <vector>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstdint>

std::vector<uint32_t> makePrefixOffline(const std::string & s) {
    std::vector<uint32_t> prefix(s.length());
    prefix[0] = 0;
    for (uint32_t i = 1; i < s.length(); ++i) {
        int j = prefix[i - 1];
        while (j > 0 && s[i] != s[j]) {
            j = prefix[j - 1];
        }
        if (s[i] == s[j]) {
            ++j;
        }
        prefix[i] = j;
    }
    return prefix;
}

void printTemplateOccurrenceOnline(std::string string_template, std::istream & input_stream, std::ostream & output_stream) {
    string_template += '$';
    uint32_t len = string_template.length() - 1;
    auto template_prefix = makePrefixOffline(string_template);

    int j = 0;
    int i = 0;
    int ch = 0;

    while (ch != EOF) {
        ch = input_stream.get();
        while (j > 0 && ch != string_template[j]) {
            j = template_prefix[j - 1];
        }
        if (ch == string_template[j]) {
            ++j;
            if (j == len) {
                output_stream << (i - len) << " ";
            }
        }
        ++i;
    }
}


int main() {
    std::ios::sync_with_stdio(false);

    std::string string_template;
    std::cin >> string_template;

//    char temp;
//    while (std::cin.get(temp) && (temp == ' ' || temp == '\n')) {}

    printTemplateOccurrenceOnline(string_template, std::cin, std::cout);
}