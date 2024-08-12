#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

class ChomskyNormalForm {
private:
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
        int countInitialAppearance = 0;
        while (getline(file, line)) {
            stringstream inputFile(line);
            string nonTerminal, arrow, production;

            inputFile >> nonTerminal >> arrow;

            if (countInitialAppearance == 0) {
                initialSymbol = nonTerminal;
            }

            while (getline(inputFile, production, '|')) {
                production.erase(0, production.find_first_not_of(" \t\n\v"));
                production.erase(production.find_last_not_of(" \t\n\v") + 1);

                grammar[nonTerminal].push_back(production);
            }
            countInitialAppearance++;
        }

        file.close();
        return true;
    }

    bool checkInitialRecursion() {
        for (const auto& rule : grammar) {
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

    void printGrammar() const {
        for (const auto& rule : grammar) {
            cout << rule.first << " -> ";
            for (size_t i = 0; i < rule.second.size(); i++) {
                cout << rule.second[i];
                if (i < rule.second.size() - 1){   
                    cout << " | ";
                }
            }
            cout << endl;
        }
    }

void removeLambda() {
    vector<string> lambdaProductions;

    // Identifica e remove produções lambda
    for (auto& rule : grammar) {
        if (rule.first != initialSymbol) {
            for (size_t i = 0; i < rule.second.size(); i++) {
                if (rule.second[i] == ".") {
                    lambdaProductions.push_back(rule.first);
                    grammar[rule.first].erase(grammar[rule.first].begin() + i);
                    i--; // Ajusta o índice para não pular produções
                }
            }
        }
    }

    // Verifica se o estado inicial (ou S') deve receber a produção " | . "
    bool initialRecursion = checkInitialRecursion();
    string initialToCheck = initialRecursion ? "S'" : initialSymbol;
    bool addLambdaToInitial = false;

    // Verifica se qualquer produção do estado inicial pode gerar uma produção lambda
    for (const auto& production : grammar[initialToCheck]) {
        if (production.find(".") == string::npos) {
            addLambdaToInitial = true;
            break;
        }
    }

    // Adiciona " | . " ao estado inicial ou a S'
    if (addLambdaToInitial) {
        grammar[initialToCheck].push_back(".");
    }
}


};

int main() {
    ChomskyNormalForm Grammar("gramatica.txt");

    if (Grammar.loadGrammar()) {
        Grammar.addStartSymbol();
        Grammar.removeLambda(); // Chama apenas removeLambda
        Grammar.printGrammar();
    } else {
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
