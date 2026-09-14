#pragma once

#include "VMEM.hpp"

#include <fstream>
#include <deque>
#include <iostream>

// la 0x1BC2 e eprocess
// 

struct record_member{
    std::string type, name, type_common, offset, attrs;
};
struct record{
    std::string id = "";
    std::string name = "";
    std::string type = "";
    uint64_t size = 0;
    std::string ref = "";
    std::string refType = "";
    std::vector<record_member> members;
    std::string modifiers = "";

    void out(){
        std::cout << "ID: " << id << ", Type: " << type << ", Size: " << size << ", Name: " << name << std::endl;
        for(auto &m : members){
            std::cout << "    Member: " << m.name << ", Type: " << m.type << ", Common Type: " << m.type_common << ", Offset: " << m.offset << ", Attrs: " << m.attrs << std::endl;
        }
    }
};

std::string l1 = "    0x1000 | LF_MODIFIER [size = 12] ";
std::string l2 = "            referent = 0x0003 (long long unsigned), modifiers = const | volatile"; 

// regex custom
// <> <- variabile, dau return
// <_> <- folosit in interior la () sau [], da return la tot basically din acest punct
// aA09! <- ignora astea daca le vezi in pattern
// <[regex]> <- ceva e in interior la [], merge cu () si ``
// exemplu:       0x1000 | LF_MODIFIER [size = 12] 
// <> | <> <[size = <>]>
// exemplu:       referent = 0x0003 (long long unsigned), modifiers = const | volatile
// <> = <> <(<_>)>, modifiers = <_>
// exemplu:         - LF_MEMBER [name = `NextFreeHandleEntry`, Type = 0x297F, offset = 8, attrs = public]
// - <> <[name = <`<_>`>, Type = <>, offset = <>, attrs = <_>]>
std::vector<std::string> parse(std::string reg, std::string line){
    std::cout << "Parsing regex: " << reg << std::endl;
    std::vector<std::string> r;
    for(int i = 0; i < reg.size(); i++){
        int c1 = reg[i];
        int c2 = reg[i + 1];
        switch(c1){
            case '<':
                switch(c2){
                    case '>':
                        // variabila normala, fara space
                        // sa verifici dacanu exista chestii dupa > (exemplu:   <>,  cu   12,  sa ret 12 si nu 12,  )
                    break;
                    case '_':
                        // returneaza cam tot de acum
                    break;
                    case '`':
                    case '(':
                    case '[':
                        // sub-regex
                    break;
                    default:
                        // nu ar trebui sa ajubnga aici
                    break;
                }
                break;
            default:
                // caracter normal, sari peste el
            break;
        }
    }
    return r;
}

void convert(){
    // std::vector<std::string> res = parse("- <> <[name = <`<_>`>, Type = <>, offset = <>, attrs = <_>]>", "      - LF_MEMBER [name = `NextFreeHandleEntry`, Type = 0x297F, offset = 8, attrs = public]");


    std::ifstream fin("assets/llvm-pdbutil.dmp");
    if (!fin.is_open()){
        std::cout << "Failed to open dump file!";
        return;
    }
    std::deque<char> line;
    std::string sline;
    record r;

    bool header = true;
    
    while (std::getline(fin, sline)){
        // header
        if (header == true && sline.find("Showing ") != std::string::npos && sline.find(" records") != std::string::npos){
            header = false;
            continue;
        }
        if (header == true) continue;
        
        // line as deque
        line.clear();
        for (char c : sline) {
            line.push_back(c);
        }

        // are spatiile alea la inceput
        while(!line.empty() && line.front() == ' ') {
            line.pop_front();
        }
        line.push_back(' '); // pt token 

        // presupunem ca e o linie pe undeva
        int sep = 0;
        while(sep < line.size() && line[sep] != '|') {
            sep++;
        }

        std::cout << "[INFO] Handling line: " << sline << std::endl;
        std::string name = "";
        for(int i = 0; i < sep; i++) name += line[i];
        uint64_t test = -1;
        try{
            test = std::stoull(name, nullptr, 16);
        }
        catch(...){
        }

        if (test == -1) {
            std::cout << "[INFO] already in a record\n";

            // sunt deja intr-un record curent
            if (r.type == "LF_MODIFIER"){
                // referent = <REF> (<TYPE W SPACE>), modifiers = <MODIFIERS W SPACE>
                std::deque<char> tok = {};
                int cnt = 0;
                for(auto i = line.begin(); i != line.end(); i++){
                    if (*i != ' ')
                        tok.push_back(*i);
                    if (*i == ' '){
                        switch(cnt){
                            case 0:
                            break;
                            case 1:
                            break;
                            case 2:
                            r.ref = std::string(tok.begin(), tok.end());
                            break;
                            case 3:
                                while(std::next(i) != line.end() && std::find(tok.begin(), tok.end(), ')') == tok.end()){
                                    tok.push_back(*(++i));
                                }
                                r.refType = std::string(std::next(tok.begin()), std::prev(std::prev(tok.end())));
                            break;
                            case 4:
                            break;
                            case 5:
                            break;
                            case 6:
                                while(std::next(i) != line.end()){
                                    tok.push_back(*(++i));
                                }
                                r.modifiers = std::string(tok.begin(), tok.end());
                            break;
                        }
                        cnt += 1;
                        tok.clear();
                    }
                }
            }else {
                std::cout << "Unhandled case: " << r.type << std::endl;
                if (!r.id.empty()){
                    std::cout << "--------- REC ---------\n";
                    r.out();
                    std::cout << "--------- ^^^ ---------\n";
                }
                return;
            }
        }else{
            // am inceput un record nou 

            if (!r.id.empty()){
                std::cout << "--------- REC ---------\n";
                r.out();
                std::cout << "--------- ^^^ ---------\n";
            }

            r = record();

            // <ID> | <TYPE> [size = <SIZE>] `<NUME_REAL>`
            std::deque<char> tok = {};
            int cnt = 0;
            for(auto &i : line){
                if (i != ' ')
                    tok.push_back(i);
                if (i == ' '){
                    switch(cnt){
                        case 0:
                            r.id = std::string(tok.begin(), tok.end());
                        break;
                        case 1:
                        break;
                        case 2:
                            r.type = std::string(tok.begin(), tok.end());
                        break;
                        case 3:
                        break;
                        case 4:
                        break;
                        case 5:
                            r.size = std::stoull(std::string(tok.begin(), std::prev(tok.end())), nullptr, 10);
                        break;
                        case 6:
                            r.name = std::string(std::next(tok.begin()), std::prev(tok.end()));
                        break;
                    }
                    cnt += 1;
                    tok.clear();
                }
            }
        }

        
    }
}