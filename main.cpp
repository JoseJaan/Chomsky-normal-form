#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class ChomskyNormalForm {
private:
    //Cria uma tabela hash;
    //Uma string, que é o estado, e um vetor de string, que são as regras
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

        //Loop para ler o arquivo
        //initialSymbol recebe o símbolo inicial da gramática
        //se countInitialAppearance==0, o primeiro símbolo está sendo lido, então ele é o inicial
        string line;
        int countIniciais = 0;
        while (getline(file, line)) {  // lê o arquivo linha por linha
            stringstream inputFile(line);
            string naoTerminal, seta, production;  // símbolos da gramática

            inputFile >> naoTerminal >> seta;

            if (countIniciais == 0) {
                simboloInicial = naoTerminal;
            }

            while (getline(inputFile, production, '|')) {
                //Apaga espaços em branco
                //\t = tabulações
                //\n = quebra de linha
                //\v = tabulação vertical
                production.erase(0, production.find_first_not_of(" \t\n\v"));  // comentar
                //+1 remove todos os caracteres após o último caractere não branco
                production.erase(production.find_last_not_of(" \t\n\v") + 1);
                //Adiciona a regra ao estado relacionado
                gramatica[naoTerminal].push_back(production);
            }
            countIniciais++;
        }

        file.close();
        return true;
    }

    //Verifica a gramática em busca de recursividade do símbolo inicial
    bool checaRecursaoInicial() {
        //Esse for é executado para cada regra da gramática
        for (const auto& regra : gramatica) {  // 'regra' é cada par de chave e valor de 'gramatica'
        //Esse for é executado para cada produção da regra
            for (const string& production : regra.second) {  // comentar (o que é second?)
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

    //Printa a gramática
    void printaGramatica() const {
        for (const auto& regra : gramatica) {
            cout << regra.first << " -> ";
            for (size_t i = 0; i < regra.second.size(); i++) {
                cout << regra.second[i];
                if (i < regra.second.size() - 1){   
                    cout << " | ";
                }
            }
            cout << endl;
        }
    }

    void removeLambda() {
        vector<string> producoesLambda;

    // Identifica e remove produções lambda
        for (auto& regra : gramatica) {
            if (regra.first != simboloInicial) {
                for (size_t i = 0; i < regra.second.size(); i++) {
                    if (regra.second[i] == ".") {
                        producoesLambda.push_back(regra.first);
                        gramatica[regra.first].erase(gramatica[regra.first].begin() + i);
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

    void aplicaRegraDaCadeia() {  // aplica a regra da cadeia à gramática
    // regra."first" é o símbolo (A, B, S...), e "second" a sua produção.

        for (auto& regra : gramatica) {  // vamos iteirar pra cada regra da gramática. o loop vai nomear essas regras V, como em "variável".
            const string& V = regra.first;
            unordered_set<string> alcancaveis;  // "alcançáveis" em relação à cadeia, não ao "reach".
            alcancaveis.insert(V);

            bool adicionou;  // "novo não-terminal já foi adicionado à alcancaveis?"

            do {
                adicionou = false;
                unordered_set<string> novosAlcancaveis;  // temp pra guardar os novos não-terminais que serão encontrados neste do while.

                for (const string& B : alcancaveis) {  // pra cada não-terminal B em alcancaveis,
                    for (const string& producao : gramatica[B]) {  // verificamos cada produção associada a B.
                        if (producao.length() == 1 && isupper(producao[0])) {  // é uma regra de cadeia (A-> B), por exemplo. (maiúsculo e tam 1)
                            if (alcancaveis.find(producao) == alcancaveis.end()) {
                                novosAlcancaveis.insert(producao);
                                adicionou = true;
                            }
                        }
                    }
                }

                alcancaveis.insert(novosAlcancaveis.begin(), novosAlcancaveis.end());  // por fim, adicionamos todos os novosAlcancaveis à alcancaveis.
            } while (adicionou);

            // copia as produções de cada não terminal em "alcancaveis" (exceto o próprio V)
            vector<string> novasProducoes;
            for (const string& B : alcancaveis) {
                if (B != V) {
                    for (const string& producao : gramatica[B]) {
                        if (!(producao.length() == 1 && isupper(producao[0]))) {
                            novasProducoes.push_back(producao);  // copiamos todo não-terminal em B para novasProducoes (exceto próprio V).
                        }
                    }
                }
            }

            // adiciona as novas produções para o símbolo V
            gramatica[V].insert(gramatica[V].end(), novasProducoes.begin(), novasProducoes.end());

            // remove produções do tipo V -> B (as de cadeia)
            gramatica[V].erase(remove_if(gramatica[V].begin(), gramatica[V].end(),   //!!! cpa q eh bom mudar isso aq (ta mto low level)
                                         [](const string& producao) {
                                             return producao.length() == 1 && isupper(producao[0]);
                                         }),
                               gramatica[V].end());
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
