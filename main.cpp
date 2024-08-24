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
            // (inclusive os próximos iniciais) a ela?
        }

        file.close();
        return true;
    }

    //Verifica a gramática em busca de recursividade do símbolo inicial
    bool checaRecursaoInicial() {
        //Esse for é executado para cada regra da gramática
        for (const auto& regra : gramatica) {  // 'regra' é cada par de chave e valor de 'gramatica'
        //Esse for é executado para cada produção da regra
            for (const string& production : regra.second) {
                if (production.find(simboloInicial)) {
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
    void printaGramatica() const {  // o-> pra que esse const depois?
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

    void removeLambdaTeste(){
        vector<string> producoesLambda;
        //Encontra regras que produzem lambda e já remove o lambda
        for (auto& regra : gramatica) {
            if (regra.first != simboloInicial) {
                for (size_t i = 0; i < regra.second.size(); i++) {
                    if (regra.second[i] == ".") {
                        producoesLambda.push_back(regra.first);
                        gramatica[regra.first].erase(gramatica[regra.first].begin() + i);  // !
                        i--; // Ajusta o índice para não pular produções
                    }
                }
            }
        }

        //Para cada regra de cada estado, verifica todos os caracteres da regra
        //Se a regra possui apenas simbolos anuláveis, o estado é anulável e adicionado no vetor
        //Toda vez que um novo estado anulável é encontrado, o loop reinicia, fazendo uma nova verificação
        bool adicionouNovasAnulaveis;
        do{
            adicionouNovasAnulaveis = false;

            for(auto& regra:gramatica){
                //Esse if verifica se o estado analisado já não é anulável
                //'find(producoesLambda.begin(),producoesLambda.end(),regra.first)' faz uma busca em 'producoesLambda' pelo estado, do inicio ao fim
                //Se nada for encontrado, o retorno de 'find' vai ser igual a 'producoesLambda.end()' e o código entra no if
                if(find(producoesLambda.begin(),producoesLambda.end(),regra.first) == producoesLambda.end()){ // if negativo
                    //para cada produção da regra
                    for(const string& producao : regra.second ){
                        bool isAnulavel = true;
                        //para cada caractere da regra
                        //se for encontrado um unico caractere terminal ou que não seja anulável, essa regra não é anulável
                        //a verificação sobre o caractere ser anulável é igual ao if de cima
                        for(char c : producao){
                            if(!isupper(c) or find(producoesLambda.begin(), producoesLambda.end(), string(1, c)) == producoesLambda.end()){
                                isAnulavel = false;
                            }
                        }
                        //se a regra possui apenas caracteres anuláveis, o estado é anulável e o adiciona no vetor
                        //o loop é reiniciado
                        if(isAnulavel){
                            producoesLambda.push_back(regra.first);
                            adicionouNovasAnulaveis = true;
                        }

                    }
                }
            }
        } while(adicionouNovasAnulaveis);

        //printa anuláveis
        cout << "Estados anuláveis: " << endl;
        for (const string& estados : producoesLambda) {
        cout << estados << " ";
        }
        cout << endl;
        cout << "================= " << endl;
       
        //para cada regra dos estados anuláveis
        //para cada caractere da regra
        //se o caractere é um simbolo anulável, gere uma nova regra sem esse símbolo
        //verifica no estado se já não existe essa nova regra
        //caso não exista, insira ela na gramática
        //reinicie a verificação das regras da gramática até o final

        //para cada regra da gramatica
        bool criouNovaProducao;
        
        do{
            criouNovaProducao = false;
            for(auto& regra : gramatica ){
                //verifica se a regra é anulável
                vector<string> novasProducoes;
                //para cada producao da regra
                for(const string& producao : regra.second){
                    //para cada caractere da produção
                    for(size_t i = 0; i < producao.size(); i++){
                        char c = producao[i];
                        //verifica se o caractere é anulável
                        if(isupper(c) and find(producoesLambda.begin(), producoesLambda.end(), string(1, c)) != producoesLambda.end()){
                            //cria uma nova producao sem o caractere anulável
                            //producao.substr(0, i) + producao.substr(i + 1); === pega a produção original e a divide em duas: de 0 até i e de i até o final, sendo i 
                            //o índice do caractere analisado. Após isso, concatena as duas novas strings em uma única string
                            string novaProducao = producao.substr(0, i) + producao.substr(i + 1);
                            //verifica se a nova produção ja nao existe,
                            //se a nova produção não é vazia, não existe na regra e já não foi inserida em 'novasProducoes'
                            if(!novaProducao.empty() and find(regra.second.begin(), regra.second.end(), novaProducao) == regra.second.end() and find(novasProducoes.begin(), novasProducoes.end(), novaProducao) == novasProducoes.end()){ // muito interessante
                                novasProducoes.push_back(novaProducao);
                                criouNovaProducao = true;
                            }
                            //se a nova produção é vazia e está sendo analisado a regra inicial, deve ser inserido o sinal de lambda
                            else if((novaProducao.empty()) and (regra.first == simboloInicial)){
                                novaProducao = '.';
                                //verifica se ja nao existe lambda e insere
                                if(find(regra.second.begin(), regra.second.end(), novaProducao) == regra.second.end() and find(novasProducoes.begin(), novasProducoes.end(), novaProducao) == novasProducoes.end()){
                                novasProducoes.push_back(novaProducao);
                                }
                            }
                        }
                    }   
                }
                //insere as novas produções na regra
                regra.second.insert(regra.second.end(), novasProducoes.begin(), novasProducoes.end());
            }
        } while(criouNovaProducao);
        //loop é executado enquanto novas regras forem criadas
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

                alcancaveis.insert(novosAlcancaveis.begin(), novosAlcancaveis.end());  // por fim, adicionamos todos os novosAlcancaveis a alcancaveis.
            } while (adicionou);

            // copia as produções de cada não terminal em "alcancaveis" (exceto o próprio V)
            vector<string> novasProducoes;
            for (const string& B : alcancaveis) {
                if (B != V) {
                    for (const string& producao : gramatica[B]) {
                        if ((!(producao.length() == 1 && isupper(producao[0]))) && (find(novasProducoes.begin(),novasProducoes.end(),producao) == novasProducoes.end())){
                            novasProducoes.push_back(producao);  // copiamos todo não-terminal em B para novasProducoes (exceto próprio V).
                        }
                    }
                }
            }
 
            // adiciona as novas produções para o símbolo V
            gramatica[V].insert(gramatica[V].end(), novasProducoes.begin(), novasProducoes.end()); // a partir de gramatica[V].end(),
            // adicionamos o intervalo de valores entre novasProducoes.begin() e end().

            // remove produções do tipo V -> B (as de cadeia)
            gramatica[V].erase(remove_if(gramatica[V].begin(), gramatica[V].end(),   //!!! cpa q eh bom mudar isso aq (ta mto low level)
                                         [](const string& producao) {
                                             return producao.length() == 1 && isupper(producao[0]);
                                         }),
                               gramatica[V].end());


        }
    }

    void aplicaReach () {
        unordered_set<string> alcancaveis;
        vector<string> pilha;

        // Inicio no símbolo inicial
        pilha.push_back(simboloInicial);
        alcancaveis.insert(simboloInicial);

        // Loop continua a rodar enquanto a pilha não estiver vazia
        while (!pilha.empty()) {
            string ElementoAtual = pilha.back(); //Pego o Elemento no topo da pilha para verificar o que ele alcança
            pilha.pop_back(); // Removo ele para processamento


            for (const string& producao : gramatica[ElementoAtual]){
                for (char simbolo : producao){
                    if (isupper(simbolo) && alcancaveis.find(string(1,simbolo)) == alcancaveis.end()) {
                        alcancaveis.insert(string(1,simbolo));
                        pilha.push_back(string(1,simbolo));
                    }
                }
            }
        }

            for (auto percorredor = gramatica.begin(); percorredor != gramatica.end();) {
                if (alcancaveis.find(percorredor->first) == alcancaveis.end()) {
                    percorredor = gramatica.erase(percorredor);
                }else {
                    percorredor++;
                }
            }
        }
};

int main() {
    ChomskyNormalForm Gramatica("gramatica.txt");

    if (Gramatica.carregaGramatica()) {
        Gramatica.adicionaSimboloInicial();
        Gramatica.removeLambdaTeste();
        Gramatica.aplicaReach();
        Gramatica.printaGramatica();
        
    } else {
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
