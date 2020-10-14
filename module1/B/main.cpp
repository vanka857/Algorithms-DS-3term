//////////////////////////////////////////////////////////////////////////////
// Problem: 1C.
//  Шаблон поиска задан строкой длины m, в которой кроме обычных символов
//  могут встречаться символы “?”.
//  Найти позиции всех вхождений шаблона в тексте длины n.
//  Каждое вхождение шаблона предполагает, что все обычные символы совпадают
//  с соответствующими из текста, а вместо символа “?” в тексте встречается
//  произвольный символ. Гарантируется, что сам “?” в тексте не встречается.
//
//  Время работы - O(n + m + Z),
//  где Z - общее число вхождений подстрок шаблона “между вопросиками”
//  в исходном тексте. m ≤ 5000, n ≤ 2000000.
//
// Пример 1:
//  Ввод:
//  ab??aba ababacaba
//  ababacaba
//  Вывод:
//  2
//
// Пример 2
//  Ввод:
//  aa??bab?cbaa? aabbbabbcbaabaabbbabbcbaab
//  aabbbabbcbaabaabbbabbcbaab
//  Вывод:
//  0 13
//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <cstdint>
#include <string>


class Trie {
private:

    struct Node {
        static const uint32_t K = 26;

        std::vector<int> next;
        std::vector<int> go;

        void make_resizing(const uint32_t alphabet_size) {
            next.resize(alphabet_size, -1);
            go.resize(alphabet_size, -1);
        }

        explicit Node(const uint32_t alphabet_size = K){
            make_resizing(alphabet_size);
        }

        Node (int parent, int char_to_parent, const uint32_t alphabet_size = K) : parent(parent), char_to_parent(char_to_parent){
            make_resizing(alphabet_size);
        }

        bool is_terminal = false;
        std::vector<int> terminal_pattern_number;
        int suffix_link = -1;
        int up_link = -1;

        char char_to_parent = -1;
        int parent = -1;

    } typedef Node;

    struct Template_t {
        std::string template_text;
        uint32_t begin_pos = 0;

        std::string toStr() const {
            std::string res;
            res += "template: " + template_text + " pos: ";
            res += std::to_string(begin_pos);
            res += "|";

            return res;
        }

        size_t length() const {
            return template_text.length();
        }
    };

    void addTemplate(const Template_t & template_t, int terminal_number = 0) {
        int v = 0;
        for (int i = 0; i < template_t.length(); ++i) {
            char c = template_t.template_text[i] - 'a';
            if (trie[v].next[c] == -1) {
                trie[v].next[c] = trie.size();
                trie.emplace_back(Node(v, c));
            }
            v = trie[v].next[c];
        }
        trie[v].is_terminal = true;
        trie[v].terminal_pattern_number.emplace_back(terminal_number);
    }

    static std::vector<Template_t> makeTemplates(const std::string & template_t) {
        std::vector<Template_t> result;

        Template_t temp;

        int j = 0;
        
        for (uint32_t i = 0; i < template_t.length(); ++i) {

            if (template_t[i] == '?' || i == template_t.length()) {
                if(!temp.template_text.empty()) {
                    temp.begin_pos = i - temp.length();
                    addTemplate(temp, j++);
                }

                temp.template_text = "";
            }
            else {
                temp.template_text += template_t[i];
            }
        }

        if(!temp.template_text.empty()) {
            temp.begin_pos = template_t.length() - temp.length();
            addTemplate(temp, j++);
        }

        return result;
    }

    

    void make(const std::vector<Template_t> & templates) {
        trie.resize(1);

        int i = 0;

//        for (const auto & template_t : templates) {
//            addTemplate(template_t, i++);
//        }
    }

    int getLink (int v) {
        if (trie[v].suffix_link == -1) {
            if (v == 0 || trie[v].parent == 0) {
                trie[v].suffix_link = 0;
            }
            else {
                trie[v].suffix_link = go(getLink(trie[v].parent), trie[v].char_to_parent);
            }
        }
        return trie[v].suffix_link;
    }

    int go (int v, char c) {
        if (trie[v].go[c] == -1) {
            if (trie[v].next[c] != -1) {
                trie[v].go[c] = trie[v].next[c];
            } else if (v == 0) {
                trie[v].go[c] = 0;
            }
            else {
                trie[v].go[c] = go(getLink(v), c);
            }
        }

        return trie[v].go[c];
    }

    int getUp(int v) {
        if (trie[v].up_link == -1) {
            auto w = getLink(v);

            if (trie[w].is_terminal || w == 0) {
                trie[v].up_link = w;
            }
            else {
                trie[v].up_link = getUp(w);
            }
        }
        return trie[v].up_link;
    }

    std::vector<Node> trie;
    std::vector<Template_t> templates;
    std::string template_t;

public:

    explicit Trie(const std::string & template_t) {
        templates = makeTemplates(template_t);
        trie.reserve(template_t.length() + 1);

        this->template_t = template_t;
        make(templates);
    }

    std::vector<int> process(const std::string & text) {
        std::vector<int> result;
        std::vector<int> counter(text.length());

        if (text.length() < template_t.length())
            return result;

        int cur = 0;

        for (int i = 0; i < text.length(); ++i) {
            char c = text[i] - 'a';

            cur = go(cur, c);

            auto a = cur;
            while (a != 0) {
                if (trie[a].is_terminal) {
                    for (auto number : trie[a].terminal_pattern_number) {
                        int pos = i + 1 - templates[number].length() - templates[number].begin_pos;

                        //std::cout << "Want to finish " << number << " template at " << pos << std::endl;

                        if (pos >= 0)
                            counter[pos]++;
                    }
                }
                a = getUp(a);
            }
        }

        for (int i = 0; i < counter.size() - template_t.length() + 1; ++i) {
            if (counter[i] == templates.size()) {
                result.push_back(i);
            }
        }

        return result;
    }

};


template <typename T>
void printVector(const std::vector<T> & data) {
    for (const auto & i : data) {
        std::cout << i.toStr() << " ";
    }
    std::cout << std::endl;
}

void printVector(const std::vector<int> & data) {
    for (const auto & i : data) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}


int main() {
    std::string template_t, text;

    std::cin >> template_t >> text;

    Trie trie(template_t);
    printVector(trie.process(text));

    return 0;
}

// aa?aa?aa?? aabaacaadd 0
// a?a?a?? abacabacd 0 2
// abac??ac abacabac 0
// ab??aba? ababacabaa 2
// ab??aba?ab ababacabacab 2
