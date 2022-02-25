#include "tokenizer.h"
#include "util.h"
#include "syntax.h"

namespace asc
{
    std::string TOKENIZER_REGEX_PATTERN;

    syntax_node* tokenize(std::ifstream& is)
    {
        syntax_node* head = new asc::syntax_node(nullptr, asc::syntax_types::PROGRAM_BEGIN, "A#", 0);
        syntax_node* current = head;
        std::string data;
        for (char c = is.get(), comment = false, in_string = false; is.good(); c = is.get())
        {
            if (c == '#' && !in_string)
                comment = true;
            if (c == '\\')
            {
                char next = is.get();
                (data += c) += next;
                continue;
            }
            if (c == '"')
                in_string = !in_string;
            if (c == '\n')
            {
                comment = false;
                data += c;
                continue;
            }
            if (comment)
                continue;
            data += c;
        }
        std::smatch sm;
        for (int line = 1, sl = 0, el = data.find('\n', sl);; line++, sl = el + 1, el = data.find('\n', sl))
        {
            if (sl >= data.length())
                break;
            if (el == std::string::npos) el = data.length() - 1;
            std::string ln = data.substr(sl, el - sl + ((el == data.length() - 1) ? 1 : 0));
            for (std::regex reg = std::regex(TOKENIZER_REGEX_PATTERN, std::regex::ECMAScript);
                std::regex_search(ln, sm, reg); ln = sm.suffix())
            {
                std::string c = sm.str();
                unsigned short t = asc::syntax_types::IDENTIFIER;
                if (is_number_literal(c))
                    t = asc::syntax_types::CONSTANT;
                else if (std::find(std::begin(STANDARD_PUNCTUATORS), std::end(STANDARD_PUNCTUATORS), c) != std::end(STANDARD_PUNCTUATORS))
                    t = asc::syntax_types::PUNCTUATOR;
                else if (is_string_literal(c))
                    t = asc::syntax_types::STRING_LITERAL;
                else if (std::find(std::begin(STANDARD_KEYWORDS), std::end(STANDARD_KEYWORDS), c) != std::end(STANDARD_KEYWORDS))
                    t = asc::syntax_types::KEYWORD;
                current = current->next = new syntax_node(nullptr, t, c, line);
            }
        }
        return head->next;
    }
}