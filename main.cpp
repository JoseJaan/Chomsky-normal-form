#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
//#include <string>

using namespace std;

int main() {
    unordered_map<string, vector<string>> grammar;
    string filename = "gramatica.txt";
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }
    
    string line;
    while (getline(file, line)) {
        
        //Pega o estado e a seta
        stringstream ss(line);
        string nonTerminal, arrow, production;
        ss >> nonTerminal >> arrow;

        while (getline(ss, production, '|')) {
            //Apaga espaços em branco
            //\t = tabulações
            //\n = quebra de linha
            //\v = tabulação vertical
            production.erase(0, production.find_first_not_of(" \t\n\v"));
            //+1 remove todos os caracteres após o último caractere não branco
            production.erase(production.find_last_not_of(" \t\n\v") + 1);
            
            //Adiciona a regra ao estado relacionado
            grammar[nonTerminal].push_back(production);
        }
    }
    
    file.close();

    //Imprime a gramática
    //'const auto& rule : grammar' => o loop vai ser executado para cada elemento dentro de grammar
    for (const auto& rule : grammar) {
        cout << rule.first << " -> ";
        for (size_t i = 0; i < rule.second.size(); ++i) {
            cout << rule.second[i];
            if (i < rule.second.size() - 1) cout << " | ";
        }
        cout << endl;
    }
    
    return 0;
}
