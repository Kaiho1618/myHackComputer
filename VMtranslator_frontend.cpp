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
    * @return 現コマンドの第1引数をstringで返す
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
     * @brief エラー時の対処。コメントとエラー行を表示して強制終了する
     * @param エラーメッセージ
    */
    void raise_error(string message){
        printf("\t>>%6d: %s\n", line, original_buffer.c_str());
        printf("%s\n", message.c_str());
        close_file();
        exit(1);
    }


    // int commandType(){
    //     size_t c;
    //     if(this->buffer[0] == '@'){
    //         return      (int)Command::A_COMMAND;
    //     }

    //     if((c = this->buffer.find_first_of(";")) != string::npos ||
    //             (c = this->buffer.find_first_of("=")) != string::npos){
    //         return (int)Command::C_COMMAND;
    //     }

    //     if(this->buffer[0] == '(' && this->buffer[this->buffer.size() - 1] == ')'){
    //         return (int)Command::L_COMMAND;
    //     }

    //     return (int)Command::INVALID_COMMAND;
    // }

    // string symbol(){
    //     int commandtype = commandType();
    //     if(commandtype == (int)Command::A_COMMAND){
    //         return buffer.substr(1, buffer.size() - 1); //先頭の@を削除
    //     }

    //     if(commandtype == (int)Command::L_COMMAND){
    //         return buffer.substr(1, buffer.size() - 2); //()を外す
    //     }

    //     return "null";
    // }


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

int main(){
    string file_path = "07/mytest01.vm";
    Parser parser(file_path);

    parser.advance();
    for(int i=1; i<10; i++){
        printf("%s\n", parser.get_argn(i).c_str());
    }
}