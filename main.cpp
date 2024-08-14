#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

class ChomskyNormalForm {
private:
    unordered_map<string, vector<string>> gramatica; 
    string filename;
    string simboloInicial;

public:
    ChomskyNormalForm(const string& filename) : filename(filename), simboloInicial("") {}

    bool carregaGramatica() {  // função pra carregar a gramática do arquivo
        ifstream file(filename);
        if (!file.is_open()) {  // checa erro no processo
            cout << "Erro ao abrir o arquivo!" << endl;
            return false;
        }

        string line;
        int countIniciais = 0;
        while (getline(file, line)) {
            stringstream inputFile(line);
            string naoTerminal, seta, production;  // símbolos da gramática

            inputFile >> naoTerminal >> seta;

            if (countIniciais == 0) {
                simboloInicial = naoTerminal;
            }

            while (getline(inputFile, production, '|')) {
                production.erase(0, production.find_first_not_of(" \t\n\v"));  // comentar
                production.erase(production.find_last_not_of(" \t\n\v") + 1);

                gramatica[naoTerminal].push_back(production);
            }
            countIniciais++;
        }

        file.close();
        return true;
    }

    bool checaRecursaoInicial() {
        for (const auto& rule : gramatica) {  // 'rule' é cada par de chave e valor de 'gramatica'
            for (const string& production : rule.second) {  // comentar (o que é second?)
                if (production.find(simboloInicial) == 1) {
                    return true;
                }
            }
        }
        return false;
    }

    void adicionaSimboloInicial() {
        if (checaRecursaoInicial()) {
            gramatica["S'"].push_back(simboloInicial);
        }
    }

    void printaGramatica() const {
        for (const auto& rule : gramatica) {
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
    vector<string> producoesLambda;

    // Identifica e remove produções lambda
    for (auto& rule : gramatica) {
        if (rule.first != simboloInicial) {
            for (size_t i = 0; i < rule.second.size(); i++) {
                if (rule.second[i] == ".") {
                    producoesLambda.push_back(rule.first);
                    gramatica[rule.first].erase(gramatica[rule.first].begin() + i);
                    i--; // Ajusta o índice para não pular produções
                }
            }
        }
    }

    // Verifica se o estado inicial (ou S') deve receber a produção " | . "
    bool initialRecursion = checaRecursaoInicial();
    string initialToCheck = initialRecursion ? "S'" : simboloInicial;
    bool addLambdaToInitial = false;

    // Verifica se qualquer produção do estado inicial pode gerar uma produção lambda
    for (const auto& production : gramatica[initialToCheck]) {
        if (production.find(".") == string::npos) {
            addLambdaToInitial = true;
            break;
        }
    }

    // Adiciona " | . " ao estado inicial ou a S'
    if (addLambdaToInitial) {
        gramatica[initialToCheck].push_back(".");
    }
}


};

int main() {
    ChomskyNormalForm Gramatica("gramatica.txt");

    if (Gramatica.carregaGramatica()) {
        Gramatica.adicionaSimboloInicial();
        Gramatica.removeLambda(); // Chama apenas removeLambda   // otavio -> cm assim?
        Gramatica.printaGramatica();
    } else {
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
