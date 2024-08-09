#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

class ChomskyNormalForm {
private:
    //Cria uma tabela hash;
    //Uma string, que é o estado, e um vetor de string, que são as regras
    unordered_map<string, vector<string>> grammar;
    string filename;
    string initialSymbol;
    
public:
    ChomskyNormalForm(const string& filename) : filename(filename), initialSymbol("") {}


    bool loadGrammar() {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Erro ao abrir o arquivo!" << endl;
            return false;
        }

        string line;

        //Loop para ler o arquivo
        //initialSymbol recebe o símbolo inicial da gramática
        //se countInitialAppearance==0, o primeiro símbolo está sendo lido, então ele é o inicial
        int countInitialAppearance = 0;
        while (getline(file, line)) {
            stringstream inputFile(line);
            string nonTerminal, arrow, production;

             //Pega o estado e a seta
            inputFile >> nonTerminal >> arrow;

            if (countInitialAppearance == 0) {
                initialSymbol = nonTerminal;
            }

            while (getline(inputFile, production, '|')) {
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
            countInitialAppearance++;
        }

        file.close();
        return true;
    }

    //Verifica a gramática em busca de recursividade do símbolo inicial
    bool checkInitialRecursion() {
        //Esse for é executado para cada regra da gramática
        for (const auto& rule : grammar) {
            //Esse for é executado para cada produção da regra
            for (const string& production : rule.second) {
                if (production.find(initialSymbol) == 1) {
                    return true;
                }
            }
        }
        return false;
    }

    void addStartSymbol() {
        if (checkInitialRecursion()) {
            grammar["S'"].push_back(initialSymbol);
        }
    }

    //Printa a gramática
    void printGrammar() const {
        for (const auto& rule : grammar) {
            cout << rule.first << " -> ";
            for (size_t i = 0; i < rule.second.size(); i++) {
                cout << rule.second[i];
                if (i < rule.second.size() - 1) cout << " | ";
            }
            cout << endl;
        }
    }
};

int main() {
    ChomskyNormalForm Grammar("gramatica.txt");

    if (Grammar.loadGrammar()) {
        Grammar.addStartSymbol();
        Grammar.printGrammar();
    }
    else{
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
