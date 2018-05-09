#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <memory>

struct Grammar
{
    std::string startSymbol;
    std::unordered_multimap<std::string, std::vector<std::string>> rules;
};

void parse(const Grammar& grammar, const std::unordered_set<std::string>& partsOfSpeech, const std::vector<std::string>& words)
{
    std::unordered_set<std::string> nonTerminals;
    for(auto it = grammar.rules.begin(); it != grammar.rules.end(); ++it)
        nonTerminals.insert(it->first);

    struct State
    {
        std::pair<std::string, std::vector<std::string>> production;
        unsigned int position;
        unsigned int start;
        unsigned int end;
        std::vector<unsigned int> hist;
            
        bool operator==(const State& rhs)
        {
            return production == rhs.production &&
                   position == rhs.position &&
                   start == rhs.start &&
                   end == rhs.end &&
                   hist == rhs.hist;
        }
    };
    
    std::vector<State> table[words.size() + 1];
    
    table[0].push_back({ *(grammar.rules.find(grammar.startSymbol)), 0, 0, 0, {} });
    
    int c = 0;

    for(unsigned int k = 0; k < words.size() + 1; k++)
    {
        for(unsigned int i = 0; i < table[k].size(); i++) // table[k] can expand during this loop
        {
            State state = table[k].at(i);
            
            if(state.position != state.production.second.size()) // if state is not finished
            {
                std::string nextElement = state.production.second.at(state.position);
                if((nonTerminals.find(nextElement)) != nonTerminals.end() &&
                   (partsOfSpeech.find(nextElement)) == partsOfSpeech.end()) 
                    // if next element of state is a non-terminal and not a part of speech
                {
                    // prediction
                    auto rules = grammar.rules.equal_range(nextElement);
                    for(auto it = rules.first; it != rules.second; ++it)
                    {
                        State s = { *it, 0, state.end, state.end, {} };
                        if(std::find(table[k].begin(), table[k].end(), s) == table[k].end())
                            table[k].push_back(s);
                    }
                }
                else
                {
                    // scanning
                    if((partsOfSpeech.find(nextElement)) != partsOfSpeech.end()) // if next element is a part of speech
                    {
                        auto rules = grammar.rules.equal_range(nextElement);
                        for(auto it = rules.first; it != rules.second; ++it)
                        {
                            if(state.end < words.size())
                            {
                                std::string word = words.at(state.end);
                                if(it->second.size() == 1 && it->second.at(0) == word)
                                {
                                    State s = { { nextElement, { word } }, 1, state.start, state.end + 1, {} };
                                    if(std::find(table[k+1].begin(), table[k+1].end(), s) == table[k+1].end())
                                        table[k+1].push_back(s);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // completion
                for(unsigned int j = 0; j < table[state.start].size(); j++)
                {
                    State otherState = table[state.start].at(j);
                    
                    std::vector<std::string> ruleRight = otherState.production.second;
                    auto it = std::find(ruleRight.begin(), ruleRight.end(), state.production.first);
                    if(it != ruleRight.end() && (std::distance(ruleRight.begin(), it) == otherState.position)) 
                    // if other state is of the right form
                    {
                        std::vector<unsigned int> hist = otherState.hist;
                        hist.push_back(c);
                        State s = { otherState.production, otherState.position + 1, otherState.start, state.end, hist };
                        if(std::find(table[k].begin(), table[k].end(), s) == table[k].end())
                            table[k].push_back(s);
                    }
                }
            }
            c++;
        }
    }
    
    int count = 0;
    std::cout << "==========================================" << std::endl;
    for(auto c : table)
    {
        for(auto s : c)
        {
            std::cout << "| " << std::setw(2) << std::left << count 
                      << " | " << std::setw(2) << std::left << s.production.first 
                      << " -> ";
            
            std::string rule;
            for(unsigned int i = 0; i < s.production.second.size(); i++)
            {
                if(i == s.position)
                    rule += "\u2022";
                else if(i > 0)
                    rule += " ";
                rule += s.production.second.at(i);
            }
            if(s.production.second.size() ==  s.position)
                rule += "\u2022";
            
            std::cout << std::setw(11) << std::left << rule << " | ";
            
            std::string bounds = "[";
            bounds += std::to_string(s.start);
            bounds += ",";
            bounds += std::to_string(s.end);
            bounds += "]";
            std::cout << std::setw(5) << std::left << bounds << " | ";
            
            std::string hist = "[";
            for(unsigned int i = 0; i < s.hist.size(); i++)
            {
                if(i > 0)
                    hist += ",";
                hist += std::to_string(s.hist.at(i));
            }
            hist += "]";
            std::cout << std::setw(7) << std::left << hist << " |";
            
            std::cout << std::endl;
            
            count++;
        }
        std::cout << "------------------------------------------" << std::endl;
    }
    std::cout << "==========================================" << std::endl;
}

int main()
{
    struct Grammar grammar;
    grammar.startSymbol = "S";
    grammar.rules = 
    {
        { "S" , { "NP", "VP" } },
        { "NP", { "N", "PP" } },
        { "NP", { "N" } },
        { "PP", { "P", "NP" } },
        { "VP", { "VP", "PP" } },
        { "VP", { "V", "PP" } },
        { "VP", { "V", "NP" } },
        { "VP", { "V" } },
        { "N" , { "they" } },
        { "N" , { "can" } },
        { "N" , { "fish" } },
        { "N" , { "rivers" } },
        { "N" , { "december" } },
        { "P" , { "in" } },
        { "V" , { "can" } },
        { "V" , { "fish" } },
    };
    /*grammar.rules = 
    {
        { "S", { "NP", "VP" } },
        { "PP", { "P", "NP" } },
        { "VP", { "V", "NP" } },
        { "VP", { "VP", "PP" } },
        { "NP", { "NP", "PP" } },
        { "NP", { "N" } },
        { "N", { "astronomers" } },
        { "N", { "ears" } },
        { "N", { "stars" } },
        { "N", { "telescopes" } },
        { "P", { "with" } },
        { "V", { "saw" } },
    };*/
    
    //parse(grammar, { "N", "V", "P" }, { "they", "can", "fish", "in", "rivers" });
    parse(grammar, { "N", "V", "P" }, { "they", "can", "fish", "in", "rivers", "in", "december" });
    
    return 0;
}
