#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <fstream>
#include <map>
#include "syntax.h"

namespace asc
{
    class tokenizer
    {
    private:
        std::ifstream* is;
        int linet;
        std::string later;
        asc::syntax_node* syntax_head;
        asc::syntax_node* current_node;
        asc::syntax_node* last_identifier;
        asc::syntax_node* scope;
    public:
        tokenizer(std::ifstream& is)
        {
            this->is = &is;
            this->linet = 1;
            this->later = "";
            this->syntax_head = new asc::syntax_node(nullptr, asc::syntax_types::PROGRAM_BEGIN, "A#", 0);
            this->current_node = this->syntax_head;
        }

        int lines()
        {
            return linet;
        }

        bool extractable()
        {
            return is->good();
        }

        tokenizer& operator>>(std::string& token)
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
                    linet++;
                if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
                    token += c;
                /// KEYWORD ///
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

        syntax_node* syntax_start()
        {
            return syntax_head->next;
        }

        std::string stringify_syntax()
        {
            std::string str = "tokenizer: syntax {";
            for (asc::syntax_node* current = this->syntax_head; current != nullptr; current = current->next)
                str += "\n\t" + current->stringify();
            return str += "\n}";
        }

        ~tokenizer()
        {
            delete syntax_head;
        }
    };
}

#endif