#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <bitset>
#include <sstream>
#include <map>

using namespace std;

enum Command{
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
    INVALID_COMMAND,
};

class Symbol{
public:
    Symbol(){
        //定義済みシンボルを定義する
        defined = {
            {"SP", 0}, {"LCL", 1}, {"ARG", 2}, {"THIS", 3}, {"THAT", 4},
            {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3},
            {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7},
            {"R8", 8}, {"R9", 9}, {"R10", 10}, {"R11", 11},
            {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15},
            {"SCREEN", 16384}, {"KBD", 24576},
        };

        address = 16;
    }

    void add_label(string symbol, int rom_address){
        label[symbol] = rom_address;
    }

    string symbol2address(string s_value){
        if(is_symbol(s_value)){
            //定義済みシンボル
            if(defined.find(s_value) != defined.end()){
                return to_string(defined[s_value]);
            }

            //ラベルシンボル
            if(label.find(s_value) != label.end()){
                return to_string(label[s_value]);
            }

            if(variables.find(s_value) == variables.end()){
                //未登録の変数
                variables[s_value] = address;
                address++;
            }
            return to_string(variables[s_value]);
        }else{
            return s_value;
        }
    }

    
    //debug用
    //labelシンボルの中身をすべて表示する
    void show_label(){
        auto begin = label.begin(), end = label.end();
        for (auto iter = begin; iter != end; iter++) {
            // first: key, second: value
            cout << "key = " << iter->first << "\n";
            cout << "value = " << iter->second << "\n";
        }
    }


private:
    map<string, int> defined; //定義済みシンボル
    map<string, int> label; //ラベルシンボル (LOOP)や(END)など
    map<string, int> variables; //変数シンボル
    int address;

    bool is_symbol(string s_value){
        if(s_value[0] - '0' <0 || s_value[0] - '0' > 9){
            return true;
        }else{
            return false; //アドレスが数値
        }
    }
};

class Parser{
public:
    Parser(string filename){
        this->filename = filename;
        open_file();
    }
    
    void open_file(){
        file.open(filename, ios::in);
        buffer = "";
        line = 0;
    }

    void close_file(){
        file.close();
    }

    bool advance(){
        size_t c;
        while(getline(file, buffer)){
            line++;
            //スペースを削除
            while((c = buffer.find_first_of(" ")) != string::npos){
                buffer.erase(c,1);
            }
            //tabを削除
            while((c = buffer.find_first_of("\t")) != string::npos){
                buffer.erase(c,1);
            }
            //改行を削除
            while((c = buffer.find_first_of("\n")) != string::npos){
                buffer.erase(c,1);
            }

            //\rを削除
            while((c = buffer.find_first_of("\r")) != string::npos){
                buffer.erase(c,1);
            }

            //コメントが含まれていれば削除
            if((c = buffer.find_first_of("//")) != string::npos){
                buffer.erase(c, buffer.size() - c);
            }

            if(buffer.size() > 0){
                return true;
            }
        }

        close_file();
        return false;

    }

    int commandType(){
        size_t c;
        if(this->buffer[0] == '@'){
            return      (int)Command::A_COMMAND;
        }

        if((c = this->buffer.find_first_of(";")) != string::npos ||
                (c = this->buffer.find_first_of("=")) != string::npos){
            return (int)Command::C_COMMAND;
        }

        if(this->buffer[0] == '(' && this->buffer[this->buffer.size() - 1] == ')'){
            return (int)Command::L_COMMAND;
        }

        return (int)Command::INVALID_COMMAND;
    }

    string symbol(){
        int commandtype = commandType();
        if(commandtype == (int)Command::A_COMMAND){
            return buffer.substr(1, buffer.size() - 1); //先頭の@を削除
        }

        if(commandtype == (int)Command::L_COMMAND){
            return buffer.substr(1, buffer.size() - 2); //()を外す
        }

        return "null";
    }

    string dest(){
        size_t c = this->buffer.find_first_of("=");
        if((int)c >= 0){
            return this->buffer.substr(0,c);
        }else{
            return "null"; //格納しない
        }

    }

    string comp(){
        size_t c_equal = this->buffer.find_first_of("=");
        size_t c_semicolon = this->buffer.find_first_of(";");
        if((int)c_equal >= 0 && (int)c_semicolon >= 0){ //ex) AM=M;JPN
            return this->buffer.substr(c_equal+1, c_semicolon - c_equal - 1);
        }
        if((int)c_equal >= 0 && (int)c_semicolon == -1){ //ex) AM=M
            return this->buffer.substr(c_equal+1, this->buffer.size() - c_equal - 1);
        }
        if((int)c_equal == -1 && (int)c_semicolon >= 0){ //ex) 0;JMP
            return this->buffer.substr(0, c_semicolon);
        }
        
        return "null"; //この場合は存在しないはず
    }

    string jump(){
        size_t c = this->buffer.find_first_of(";");
        if((int)c == -1){
            return "null";
        }
        return this->buffer.substr(c+1, this->buffer.size() - c);
    }


    void debug(){
        while(advance()){
            printf("line:%d, size:%ld", line, buffer.size());
            cout << "[" << buffer << "]" << endl;
            int commandtype = commandType();
            printf("commandtype:%d, ", commandtype);
            if(commandtype == (int)Command::A_COMMAND || commandtype == (int)Command::L_COMMAND){
                printf("symbol[%s],", symbol().c_str());
            }

            if(commandtype == (int)Command::C_COMMAND){
                printf("dest[%s], comp[%s],jump[%s]", dest().c_str(), comp().c_str(), jump().c_str());
            }
            printf("\n\n");
        }
    }

    void show_command(){
        printf("line%d: %s\n", line, buffer.c_str());
    }

private:
    int line;
    string filename;
    ifstream file;
    string buffer;
};


class Code{
public:
    Code(){
        mp_comp["0"]    = "0101010";
        mp_comp["1"]    = "0111111";
        mp_comp["-1"]   = "0111010";
        mp_comp["D"]    = "0001100";
        mp_comp["A"]    = "0110000";
        mp_comp["!D"]   = "0001101";
        mp_comp["!A"]   = "0110001";
        mp_comp["-D"]   = "0001111";
        mp_comp["-A"]   = "0110011";
        mp_comp["D+1"]  = "0011111";
        mp_comp["A+1"]  = "0110111";
        mp_comp["D-1"]  = "0001110";
        mp_comp["A-1"]  = "0110010";
        mp_comp["D+A"]  = "0000010";
        mp_comp["D-A"]  = "0010011";
        mp_comp["A-D"]  = "0000111";
        mp_comp["D&A"]  = "0000000";
        mp_comp["D|A"]  = "0010101";
        mp_comp["M"]    = "1110000";
        mp_comp["!M"]   = "1110001";
        mp_comp["-M"]   = "1110011";
        mp_comp["M+1"]  = "1110111";
        mp_comp["M-1"]  = "1110010";
        mp_comp["D+M"]  = "1000010";
        mp_comp["D-M"]  = "1010011";
        mp_comp["M-D"]  = "1000111";
        mp_comp["D&M"]  = "1000000";
        mp_comp["D|M"]  = "1010101";

        mp_dest["null"] = "000";
        mp_dest["M"]    = "001";
        mp_dest["D"]    = "010";
        mp_dest["MD"]   = "011";
        mp_dest["A"]    = "100";
        mp_dest["AM"]   = "101";
        mp_dest["AD"]   = "110";
        mp_dest["AMD"]  = "111";

        mp_jump["null"] = "000";
        mp_jump["JGT"]  = "001";
        mp_jump["JEQ"]  = "010";
        mp_jump["JGE"]  = "011";
        mp_jump["JLT"]  = "100";
        mp_jump["JNE"]  = "101";
        mp_jump["JLE"]  = "110";
        mp_jump["JMP"]  = "111";
    }

    string a_order(string s_value){
        int value = atoi(s_value.c_str());
        stringstream bit_value;
        bit_value << std::bitset<15>(value);

        string bit_order = "0" + bit_value.str();
        return bit_order;
    } 

    string c_order(string comp, string dest, string jump){
        string bit_order;
        

        if(mp_comp.count(comp) && mp_dest.count(dest) && mp_jump.count(jump)){
            bit_order = "111" + mp_comp[comp] + mp_dest[dest] + mp_jump[jump];
        }else{
            bit_order = "";
        }
        return bit_order;
    }

private:
    map<string, string> mp_comp;
    map<string, string> mp_dest;
    map<string, string> mp_jump;

};


  

int main(int argc, char *argv[]){
    string save_file = "Prog.hack";
    Parser parser("pong/Pong.asm");
    Code code;
    Symbol symbol;
    
    //1回目のパス
    int rom_address = 0;
    while(parser.advance()){
        int commandtype = parser.commandType();
        if(commandtype == Command::L_COMMAND){
            symbol.add_label(parser.symbol(), rom_address);
        }
        if(commandtype == Command::A_COMMAND || commandtype == Command::C_COMMAND){
            rom_address++;
        }
    }
    parser.close_file();


    //2回目のパス
    parser.open_file();
    std::ofstream f;
    f.open(save_file, std::ios::out);
    while(parser.advance()){
        string order;
        int commandtype = parser.commandType();
        
        if(commandtype == Command::L_COMMAND){
            continue;
        }

        if(commandtype == Command::A_COMMAND){
            string s_value = parser.symbol();
            string address = symbol.symbol2address(s_value);
            
            order = code.a_order(address);
        }
        if(commandtype == Command::C_COMMAND){
            string dest, comp, jump;
            dest = parser.dest();
            comp = parser.comp();
            jump = parser.jump();
            order = code.c_order(comp, dest, jump);
        }

        //無効なコマンド
        if(order==""){
            printf("[ERROR]INVALID COMMAND:\n");
            parser.show_command();
            f.close();
            return 0;
        }

        f << order << endl;
        
    }

    f.close();
}