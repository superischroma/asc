#include "tokenizer.h"
#include "util.h"

namespace asc
{
    const unsigned long long COMMENT = 1 << 0;
    const unsigned long long STRING_LITERAL = 1 << 1;

    tokenizer::tokenizer(std::ifstream& is)
    {
        this->is = &is;
        this->linet = 1;
        this->later = "";
        this->syntax_head = new asc::syntax_node(nullptr, asc::syntax_types::PROGRAM_BEGIN, "A#", 0);
        this->current_node = this->syntax_head;
        this->options = 0ULL;
    }

    int tokenizer::lines()
    {
        return linet;
    }

    bool tokenizer::extractable()
    {
        return is->good();
    }

    tokenizer& tokenizer::operator>>(std::string& token)
    {
        token.clear();
        if (this->later.length() != 0)
        {
            token = this->later;
            this->later.erase();
            return *this;
        }
        for (char c = is->get(); is->good(); c = is->get())
        {
            if (c == '\n')
            {
                linet++;

                //// COMMENTS ////
                if ((this->options & COMMENT) != 0) // if one line comment
                    this->options &= ~COMMENT; // get rid of comment
            }
            if ((this->options & COMMENT) != 0) // in a comment
                continue;
            if (token == "#") // if token is comment
            {
                this->options |= COMMENT;
                token.clear();
                continue;
            }
            
            //// STRING LITERALS ////
            if (c == '"' && !asc::ends_with(token, "\\")) // if we encounter a non-escaped double quote
            {
                if ((this->options & STRING_LITERAL) == 0) // if we're not in a literal
                    this->options |= STRING_LITERAL; // put us in a literal
                else // otherwise
                {
                    this->options &= ~STRING_LITERAL; // kick us out of the literal
                    token += c; // add the ending quote
                    // make a new syntax node
                    this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::STRING_LITERAL, token, linet);
                    return *this;
                }
            }
            if ((this->options & STRING_LITERAL) != 0) // in a string iteral
            {
                token += c;
                continue; // don't check for keywords, punctuators, etc.
            }

            if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
                token += c;
            //// KEYWORD ////
            if (asc::is_keyword(token))
            {
                this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::KEYWORD, token, linet);
                return *this;
            }
            //// PUNCTUATOR ////
            if (unsigned char len = asc::is_punctuator(token))
            {
                if (token.length() == len) // if it's just the punctuator
                {
                    this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::PUNCTUATOR, token, linet);
                    return *this;
                }
                // if not, separate the punctuator from the other token, and save the punctuator for later
                later = token.substr(token.length() - len);
                token = token.substr(0, token.length() - len);
                // punctuator is stored in later, let's find the type of the token we're on right now
                if (asc::is_numerical(token)) // is it numerical?
                    // add it as a constant
                    this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::CONSTANT, token, linet);
                else // is it an identifier?
                    // add it as one
                    this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::IDENTIFIER, token, linet);
                this->current_node = this->current_node->next = new asc::syntax_node(nullptr, asc::syntax_types::PUNCTUATOR, later, linet);
                return *this;
            }
        }
        return *this;
    }

    syntax_node* tokenizer::syntax_start()
    {
        return syntax_head->next;
    }

    std::string tokenizer::stringify_syntax()
    {
        std::string str = "tokenizer: syntax {";
        for (asc::syntax_node* current = this->syntax_head; current != nullptr; current = current->next)
            str += "\n\t" + current->stringify();
        return str += "\n}";
    }

    tokenizer::~tokenizer()
    {
        delete syntax_head;
    }
}