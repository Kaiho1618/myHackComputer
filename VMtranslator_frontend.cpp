/**
 * @brief 7章バーチャルマシン#1. VMファイルをアセンブラコードに変換する
 * @date　2023年03月08日
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <bitset>
#include <sstream>
#include <map>
#include<vector>


using namespace std;

enum Command{
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
};

class Arithmetic_Command{
public:
    const string ADD = "add";
    const string SUB = "sub";
    const string NEG = "neg";
    const string EQ = "eq";
    const string GT = "gt";
    const string LT = "lt";
    const string AND = "and";
    const string OR = "or";
    const string NOT = "not";
   
    /**
     * @brief 算術コマンドかの判定
     * @param コマンド
    */
    bool is_arithmetic_command(string command){
        bool is_arithmetic = (command == ADD) ||  (command == SUB) ||  (command == NEG)||  (command == EQ)
                            ||  (command == GT)||  (command == LT)||  (command == AND)||  (command == OR)||  (command == NOT);
        return is_arithmetic;
    }
}arithmetic_command;


/***
 * @brief RAMアドレスの割り当て
*/
enum RAM_Address{
    register_begin = 0,
    register_end = 15,
    static_variable_begin = 16,
    static_variable_end = 255,
    stack_begin = 256,
    stack_end = 2047,
    heap_begin = 2048,
    heap_end = 16383,
    io_begin = 16384,
    io_end = 24575,
    unused_begin = 24576,
    unused_end = 32767,
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


    /**
     * @brief 次のコマンドをbufferに入れる
     * @return 次のコマンドが存在すればtrueを、しなければfalseを返す
     * @detail コメントや\t,\nなどが取り除かれた、必要十分なコマンドのみがbufferに入る
     */
    bool advance(){
        size_t c;
        while(getline(file, buffer)){
            line++;
            original_buffer = buffer;

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

    /**
    * @brief 現コマンド(現在bufferに入っているコマンド)のn個目の引数を取得する
    * @return 現コマンドの第n引数をstringで返す
    **/
    string get_argn(int n){
        if(n == 0){
            string error_message = "argument0 is invalid";
            raise_error(error_message);
        }

        string s;
        
        int i=0;
        stringstream ss{buffer};
        while(getline(ss, s, ' ')){
            i++;
            if(i == n){
                return s;
            }
        }

        string error_message = "the arguement " + to_string(n) + " is designed, but this command has " + to_string(i) + " arguments";
        raise_error(error_message);
        return "";
    }

    /**
    * @brief 現コマンド(現在bufferに入っているコマンド)のn個目の引数を取得しintで返す
    * @return 現コマンドの第n引数をintで返す
    **/
    int get_argn_int(int n){
        string arg = get_argn(n);
        return stoi(arg);
    }

    

    /**
     * @brief エラー時の対処。コメントとエラー行を表示して強制終了する
     * @param エラーメッセージ
    */
    void raise_error(string message){
        printf("\t>>%6d: %s\n", line, original_buffer.c_str());
        printf("%s\n", message.c_str());
        close_file();
        exit(1);
    }

    /***
     * @brief VMコマンドの種類を返す。算術コマンドはすべてC_ARITHMETICが返される
     * @return Commandによって割り当てられているintの値
    */

    int commandType(){
        string arg1 = get_argn(1);
        bool is_arithmetic = arithmetic_command.is_arithmetic_command(arg1);
        
        if(is_arithmetic){
            return Command::C_ARITHMETIC;
        }else if(arg1 == "push"){
            return Command::C_PUSH;
        }else if(arg1 == "pop"){
            return Command::C_POP;
        }else if(arg1 == "label"){
            return Command::C_LABEL;
        }else if(arg1 == "goto"){
            return Command::C_GOTO;
        }else if(arg1 == "if"){
            return Command::C_IF;
        }else if(arg1 == "function"){
            return Command::C_FUNCTION;
        }else if(arg1 == "return"){
            return Command::C_RETURN;
        }else if(arg1 == "call"){
            return Command::C_CALL;
        }

        string message = "Invalid command type";
        raise_error(message);
        return -1;
    }

    // void debug(){
    //     while(advance()){
    //         printf("line:%d, size:%ld", line, buffer.size());
    //         cout << "[" << buffer << "]" << endl;
    //         int commandtype = commandType();
    //         printf("commandtype:%d, ", commandtype);
    //         if(commandtype == (int)Command::A_COMMAND || commandtype == (int)Command::L_COMMAND){
    //             printf("symbol[%s],", symbol().c_str());
    //         }

    //         if(commandtype == (int)Command::C_COMMAND){
    //             printf("dest[%s], comp[%s],jump[%s]", dest().c_str(), comp().c_str(), jump().c_str());
    //         }
    //         printf("\n\n");
    //     }
    // }

    void show_command(){
        printf("line%d: %s\n", line, buffer.c_str());
    }

private:
    int line;
    string filename;
    ifstream file;
    string buffer;
    string original_buffer; //コメント等が削除されていない状態のbuffer
};



/**
 * @brief VMコマンドをHackアセンブリコードに変換する
*/
class CodeWriter{
public:
    /**
     * @brief 出力ファイルを開き、書き込む準備をする
     * @param 出力ファイルのパス
    */
    CodeWriter(string save_file){
        f.open(save_file, ios::out);

        static_variable_index = RAM_Address::static_variable_begin;
        stack_index = RAM_Address::stack_begin;
        heap_index = RAM_Address::heap_begin;
    }


    /**
     * @brief 出力ファイルを閉じる
    */
    void close(){
        f.close();
        return;
    }

    /**
     * @brief RAM[n]のデータをレジスタregisterにロードする
     * @param n:RAMのアドレス
     * @param R: ロード先のレジスタ。A, M, D, MD, AM, AD, AMDのいずれか
    */
    void load_RAM(int n, string R){
        string asm_code = "";
        asm_code += "@" + to_string(n) + "\n"; 
        
        if(R != "M"){
            asm_code += R + "=M" + "\n";
        }

        f << asm_code;
    }

    /**
     * @brief 1変数関数の算術コマンドをアセンブリコマンドに変換する
     * @param command: アセンブリコマンドの演算子。
    */
    void arithmetic_1_variable(string command){
        string asm_command;
        asm_command = "M = " + command + "M\n";
        f << asm_command;
    }

    /**
     * @brief 2変数関数の算術コマンドをアセンブリコマンドに変換する
     * @param command: アセンブリコマンドの演算子。
     * @todo yがすでにレジスタMにロードされていることを想定している。
    */
    void arithmetic_2_variable(string command){
        load_RAM(stack_index, "D");
        load_RAM(stack_index, "M");
        
        string asm_command;
        asm_command = "M = M" + command + "D\n";
        f << asm_command;
    }


    /**
     * @brief 与えられた算術コマンドをアセンブラコードに変換し、それを書き込む
     * @param 算術コマンド
    */
    void writeArithmetic(string command){
        if(command == arithmetic_command.ADD){
            arithmetic_2_variable("+");
        }else if(command == arithmetic_command.SUB){
            arithmetic_2_variable("-");
        }else if(command == arithmetic_command.NEG){
            arithmetic_1_variable("|");
        }else if(command == arithmetic_command.EQ){

        }else if(command == arithmetic_command.GT){
            
        }else if(command == arithmetic_command.LT){
            
        }else if(command == arithmetic_command.AND){
            arithmetic_2_variable("&");
        }else if(command == arithmetic_command.OR){
            arithmetic_2_variable("|");
        }else if(command == arithmetic_command.NOT){
            arithmetic_1_variable("!");
        }
    }



private:
    ofstream f;

    //使用しているRAMのアドレス
    int static_variable_index; //スタティック変数のindex. 16-255
    int stack_index; //スタック. 256-2047
    int heap_index; //ヒープ(オブジェクトと配列を格納する). 2048-16383

};

int main(){
    string file_path = "07/mytest01.vm";
    Parser parser(file_path);

    while(parser.advance()){
        printf("%d\n", parser.commandType());
    }
}