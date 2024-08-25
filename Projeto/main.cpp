#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

using namespace std;

class ChomskyNormalForm
{
private:
    //A gramatica é armazenada em um tabela hash
    //Uma string, que é a regra, e um vetor de string, que são as produções
    unordered_map<string, vector<string>> gramatica;
    string filename;
    string simboloInicial, simboloInicialAntigo; //Utilizado apenas para printar
    bool adicionouSimboloInicial = false; //Utilizado apenas para printar

public:
    ChomskyNormalForm(const string &filename) : filename(filename), simboloInicial("") {}

    //Ler o arquivo
    bool carregaGramatica()
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "Erro ao abrir o arquivo!" << endl;
            return false;
        }

        // Loop para ler o arquivo
        // initialSymbol recebe o símbolo inicial da gramática
        // se countInitialAppearance==0, o primeiro símbolo está sendo lido, então ele é o inicial
        string line;
        int countIniciais = 0;
        // lê o arquivo linha por linha
        while (getline(file, line))
        { 
            stringstream inputFile(line);
            string naoTerminal, seta, production; // símbolos da gramática

            inputFile >> naoTerminal >> seta;

            if (countIniciais == 0)
            {
                simboloInicial = naoTerminal;
            }

            while (getline(inputFile, production, '|'))
            {
                // Apaga espaços em branco
                //\t = tabulações
                //\n = quebra de linha
                //\v = tabulação vertical
                production.erase(0, production.find_first_not_of(" \t\n\v")); // comentar
                //+1 remove todos os caracteres após o último caractere não branco
                production.erase(production.find_last_not_of(" \t\n\v") + 1);
                // Adiciona a regra ao estado relacionado
                gramatica[naoTerminal].push_back(production);
            }
            countIniciais++;
        }

        file.close();
        return true;
    }

    // Verifica a gramática em busca de recursividade do símbolo inicial
    bool checaRecursaoInicial()
    {
        //Executado para cada regra da gramática
        for (const auto &regra : gramatica)
        {
            //Executado para cada produção da regra
            for (const string &production : regra.second)
            {
                //Executado para cada char na producao
                for(char caractere : production){
                    string stringCaractere(1, caractere);
                    
                    //Se char == simboloInicial, há recursão
                    if(stringCaractere == simboloInicial){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    //Se houver recursão do simbolo inicial, adiciona um novo simbolo inicial
    void adicionaSimboloInicial()
    {
        if (checaRecursaoInicial())
        {
            gramatica["S'"].push_back(simboloInicial);
            simboloInicialAntigo = simboloInicial;
            simboloInicial = "S'";
            adicionouSimboloInicial = true;
        }
    }

    void printaGramatica() const
    {
        //Imprime a regra do símbolo inicial
        auto inicial = gramatica.find(simboloInicial);
        if (inicial != gramatica.end())
        {
            cout << inicial->first << " -> ";
            for (size_t i = 0; i < inicial->second.size(); i++)
            {
                cout << inicial->second[i];
                if (i < inicial->second.size() - 1)
                {
                    cout << " | ";
                }
            }
            cout << endl;
        }
        //Imprime a regra do antigo simbolo inicial, se houver, para manter a ordem da impressão
        if(adicionouSimboloInicial == true)
        {
            auto regra = gramatica.find(simboloInicialAntigo);
            if (regra != gramatica.end())
            {
                cout << regra->first << " -> ";
                for (size_t i = 0; i < regra->second.size(); i++)
                {
                    cout << regra->second[i];
                    if (i < regra->second.size() - 1)
                    {
                        cout << " | ";
                    }
                }
                cout << endl;
            }
        }

        // Cria um map para armazenar as outras regras em ordem alfabética
        map<string, vector<string>> gramaticaOrdenada(gramatica.begin(), gramatica.end());

        // Imprime as demais regras, exceto a do símbolo inicial e do símbolo inicial antigo
        for (const auto &regra : gramaticaOrdenada)
        {
            if (regra.first != simboloInicial && regra.first != simboloInicialAntigo)
            {
                cout << regra.first << " -> ";
                for (size_t i = 0; i < regra.second.size(); i++)
                {
                    cout << regra.second[i];
                    if (i < regra.second.size() - 1)
                    {
                        cout << " | ";
                    }
                }
                cout << endl;
            }
        }
    }
    // Encontra regras que produzem lambda e já o remove
    void removeLambda()
    {
        vector<string> producoesLambda;
        for (auto &regra : gramatica)
        {
            if (regra.first != simboloInicial)
            {
                for (size_t i = 0; i < regra.second.size(); i++)
                {
                    if (regra.second[i] == ".")
                    {
                        producoesLambda.push_back(regra.first);
                        gramatica[regra.first].erase(gramatica[regra.first].begin() + i);
                        //Ajusta o índice para não pular produções 
                        i--;                                                              
                    }
                }
            }
        }

        // Para cada producao de cada regra, verifica todos os caracteres da producao
        // Se a producao possui apenas simbolos anuláveis, a regra é anulável e adicionada no vetor
        // Toda vez que uma nova regra anulável é encontrado, o loop reinicia, fazendo uma nova verificação
        bool adicionouNovasAnulaveis;
        do
        {
            adicionouNovasAnulaveis = false;

            for (auto &regra : gramatica)
            {
                // Esse if verifica se o estado analisado já não é anulável
                if (find(producoesLambda.begin(), producoesLambda.end(), regra.first) == producoesLambda.end())
                {
                    for (const string &producao : regra.second)
                    {
                        bool isAnulavel = true;
                        // se for encontrado um unico caractere terminal ou que não seja anulável, essa regra não é anulável
                        for (char c : producao)
                        {
                            if (!isupper(c) || find(producoesLambda.begin(), producoesLambda.end(), string(1, c)) == producoesLambda.end())
                            {
                                isAnulavel = false;
                            }
                        }
                        // se a regra possui apenas caracteres anuláveis, o estado é anulável e o adiciona no vetor
                        // o loop é reiniciado
                        if (isAnulavel)
                        {
                            producoesLambda.push_back(regra.first);
                            adicionouNovasAnulaveis = true;
                        }
                    }
                }
            }
        } while (adicionouNovasAnulaveis);

        // para cada regra dos estados anuláveis
        // para cada caractere da regra
        // se o caractere é um simbolo anulável, gere uma nova regra sem esse símbolo
        // verifica no estado se já não existe essa nova regra
        // caso não exista, insira ela na gramática
        // reinicie a verificação das regras da gramática até o final

        bool criouNovaProducao;
        do
        {
            criouNovaProducao = false;
            for (auto &regra : gramatica)
            {
                vector<string> novasProducoes;
                for (const string &producao : regra.second)
                {
                    for (size_t i = 0; i < producao.size(); i++)
                    {
                        char c = producao[i];
                        // verifica se o caractere é anulável
                        if (isupper(c) && find(producoesLambda.begin(), producoesLambda.end(), string(1, c)) != producoesLambda.end())
                        {
                            // cria uma nova producao sem o caractere anulável
                            // producao.substr(0, i) + producao.substr(i + 1); === pega a produção original e a divide em duas: de 0 até i e de i até o final, sendo i
                            // o índice do caractere analisado. Após isso, concatena as duas novas strings em uma única string
                            string novaProducao = producao.substr(0, i) + producao.substr(i + 1);
                            // verifica se a nova produção ja nao existe,
                            // se a nova produção não é vazia, não existe na regra e já não foi inserida em 'novasProducoes'
                            if (!novaProducao.empty() && find(regra.second.begin(), regra.second.end(), novaProducao) == regra.second.end() && find(novasProducoes.begin(), novasProducoes.end(), novaProducao) == novasProducoes.end())
                            {
                                novasProducoes.push_back(novaProducao);
                                criouNovaProducao = true;
                            }
                            // se a nova produção é vazia e está sendo analisado a regra inicial, deve ser inserido o sinal de lambda
                            else if ((novaProducao.empty()) && (regra.first == simboloInicial))
                            {
                                novaProducao = '.';
                                if (find(regra.second.begin(), regra.second.end(), novaProducao) == regra.second.end() && find(novasProducoes.begin(), novasProducoes.end(), novaProducao) == novasProducoes.end())
                                {
                                    novasProducoes.push_back(novaProducao);
                                }
                            }
                        }
                    }
                }
                // insere as novas produções na regra
                regra.second.insert(regra.second.end(), novasProducoes.begin(), novasProducoes.end());
            }
        } while (criouNovaProducao);
    }

    //Regra da cadeia
    void aplicaRegraDaCadeia()
    {
        for (auto &regra : gramatica)
        { // vamos iteirar pra cada regra da gramática. o loop vai nomear essas regras V, como em "variável".
            const string &V = regra.first;
            unordered_set<string> alcancaveis; // "alcançáveis" em relação à cadeia, não ao "reach".
            alcancaveis.insert(V);

            bool adicionou;

            do
            {
                adicionou = false;
                unordered_set<string> novosAlcancaveis; // temp pra guardar os novos não-terminais que serão encontrados neste do while.

                for (const string &B : alcancaveis)
                { // pra cada não-terminal B em alcancaveis,
                    for (const string &producao : gramatica[B])
                    { // verificamos cada produção associada a B.
                        if (producao.length() == 1 && isupper(producao[0]))
                        { // é uma regra de cadeia (A-> B), por exemplo. (maiúsculo e tam 1)
                            if (alcancaveis.find(producao) == alcancaveis.end())
                            {
                                novosAlcancaveis.insert(producao);
                                adicionou = true;
                            }
                        }
                    }
                }

                alcancaveis.insert(novosAlcancaveis.begin(), novosAlcancaveis.end()); // por fim, adicionamos todos os novosAlcancaveis a alcancaveis.
            } while (adicionou);

            // copia as produções de cada não terminal em "alcancaveis" (exceto o próprio V)
            vector<string> novasProducoes;
            for (const string &B : alcancaveis)
            {
                if (B != V)
                {
                    for (const string &producao : gramatica[B])
                    {
                        if ((!(producao.length() == 1 && isupper(producao[0]))) && (find(novasProducoes.begin(), novasProducoes.end(), producao) == novasProducoes.end()))
                        {
                            novasProducoes.push_back(producao); // copiamos todo não-terminal em B para novasProducoes (exceto próprio V).
                        }
                    }
                }
            }

            // adiciona as novas produções para o símbolo V
            gramatica[V].insert(gramatica[V].end(), novasProducoes.begin(), novasProducoes.end()); // a partir de gramatica[V].end(),
            // adicionamos o intervalo de valores entre novasProducoes.begin() e end().

            // remove produções do tipo V -> B (as de cadeia)
            gramatica[V].erase(remove_if(gramatica[V].begin(), gramatica[V].end(), //!!! cpa q eh bom mudar isso aq (ta mto low level)
                                         [](const string &producao)
                                         {
                                             return producao.length() == 1 && isupper(producao[0]);
                                         }),
                               gramatica[V].end());
        }
    }

    void aplicaReach()
    {
        unordered_set<string> alcancaveis;
        vector<string> pilha;

        // Inicio no símbolo inicial
        pilha.push_back(simboloInicial);
        alcancaveis.insert(simboloInicial);
        // Loop continua a rodar enquanto a pilha não estiver vazia
        while (!pilha.empty())
        {
            string ElementoAtual = pilha.back(); // Pego o Elemento no topo da pilha para verificar o que ele alcança
            pilha.pop_back();                    // Removo ele para processamento

            for (const string &producao : gramatica[ElementoAtual])
            {
                for (char simbolo : producao)
                {
                    if (isupper(simbolo) && alcancaveis.find(string(1, simbolo)) == alcancaveis.end())
                    {
                        alcancaveis.insert(string(1, simbolo));
                        pilha.push_back(string(1, simbolo));
                    }
                }
            }
        }

        for (auto percorredor = gramatica.begin(); percorredor != gramatica.end();)
        {
            if (alcancaveis.find(percorredor->first) == alcancaveis.end())
            {
                percorredor = gramatica.erase(percorredor);
            }
            else
            {
                percorredor++;
            }
        }
    }

    void aplicaTerm()
    {
        // para cada regra da gramática
        // para cada produção da regra
        // se a produção for terminal diretamente (a,b,c), o estado é terminal e adicionado em 'terminais'
        vector<string> terminais;
        for (auto &regra : gramatica)
        {
            for (const string &producao : regra.second)
            {
                if (producao.length() == 1 && !isupper(producao[0]) && find(terminais.begin(), terminais.end(), regra.first) == terminais.end())
                {
                    terminais.push_back(regra.first);
                }
            }
        }
        // encontrar terminais indiretos
        // reiniciar o loop toda vez q um novo terminal for inserido
        // verifica cada caractere da produção
        // se todos os caracteres da produção forem minúsculos ou terminais, o estado é terminal
        bool encontrouNovoTerminal;
        do
        {
            encontrouNovoTerminal = false;
            for (auto &regra : gramatica)
            {

                for (const string &producao : regra.second)
                {
                    bool eTerminal = true;
                    for (const char caractere : producao)
                    {
                        string stringCaractere(1, caractere);
                        if (isupper(caractere) && find(terminais.begin(), terminais.end(), stringCaractere) == terminais.end())
                        {
                            eTerminal = false;
                        }
                    }
                    if (eTerminal)
                    {
                        if (find(terminais.begin(), terminais.end(), regra.first) == terminais.end())
                        {
                            terminais.push_back(regra.first);
                            encontrouNovoTerminal = true;
                        }
                    }
                }
            }
        } while (encontrouNovoTerminal == true);

        // para cada regra.first da gramática
        // se regra.first não estiver em 'terminais', é removida
        // se regra.second referencia símbolo não-contido em "terminais"
        // regra.second é removida da regra.first
        vector<string> estadosParaRemover;
        for (auto &regra : gramatica)
        {
            if (find(terminais.begin(), terminais.end(), regra.first) == terminais.end())
            {  
                estadosParaRemover.push_back(regra.first);  
            }
            else
            {
                for (size_t i = 0; i < regra.second.size(); i++)
                {
                    for(const char c : regra.second[i]){
                        string stringCaractere(1, c);
                        if(isupper(c) && find(terminais.begin(), terminais.end(), stringCaractere) == terminais.end()){
                            gramatica[regra.first].erase(gramatica[regra.first].begin() + i);
                        }
                    }

                }
            }
        }

        for (const string& estado : estadosParaRemover)
        {
            gramatica.erase(estado);
        }

    }

    void aplicaChomsky()
    {
   
        //Para nomear regras
        int numeroRegra = 1;

        //Deixa todas as produções com tamanho 2
        bool criouEstadoNovo;
        do{
            criouEstadoNovo = false;
            for (auto &regra : gramatica)
            {
                for (string &producao : regra.second)
                {
                    if (producao.length() > 2)
                    {
                        //Armazena em uma variável o conteúdo da produção a partir do 2° caractere
                        //Remove da produção todos os caracteres a partir do 2°
                        bool existeProducao = false;
                        string novaProducao = producao.substr(1);
                        producao.erase(1);
                        
                        for(auto& regraVerificacao: gramatica)
                        {
                            if(regraVerificacao.second.size() == 1){
                                for(const string &producaoVerificacao : regraVerificacao.second)
                                {   
                                    //Se existe uma regra que possui apenas 1 produção
                                    //Essa produção for igual à 'novaProdução'
                                    //Adicionamos à produção origial o símbolo dessa regra (concatena)
                                    if(producaoVerificacao == novaProducao){
                                        existeProducao = true;
                                        producao += regraVerificacao.first;
                                    }
                                }
                            }
                        }
                        if(existeProducao == false){
                            string nomeRegra = "R" + to_string(numeroRegra);
                            gramatica[nomeRegra].push_back(novaProducao);
                            producao += nomeRegra;
                            numeroRegra++;
                        }
                    }
                }
            }
        }while(criouEstadoNovo == true);

        bool criouNovaRegra = true;
        do{
            criouNovaRegra = false;
            for(auto &regra : gramatica)
            {
                for(string &producao : regra.second)
                {   
                    vector<string> simbolos;
                    string simboloAtual;

                    //Separa a producao em simbolos
                    for(char &caractere : producao)
                    {   
                        //se o caractere é maiúsculo ou se é um dígito 
                        //para ser um dígito, obrigatoriamente ele leu outro caractere antes, que está armazenado em simboloAtual
                        if(isupper(caractere) || (isdigit(caractere) && !simboloAtual.empty()))
                        {
                            simboloAtual += caractere;
                        }
                        else
                        {
                            if(!simboloAtual.empty()) 
                            {
                                simbolos.push_back(simboloAtual);
                                simboloAtual.clear();
                            }
                            simboloAtual += caractere;
                            simbolos.push_back(simboloAtual);
                            simboloAtual.clear();
                        }
                    }
                    if(!simboloAtual.empty()) 
                    {
                        simbolos.push_back(simboloAtual);
                    }

                    if(simbolos.size() == 2)
                    {
                        string novaProducao;
                        for(string &simbolo : simbolos)
                        {
                            if(simbolo.size() == 1 && !isupper(simbolo[0]))
                            {
                                bool achouProducaoIgual = false;
                                //verificar se ja existe uma regra que tem uma produção que produz apenas esse terminal
                                //caso negativo, criar uma nova regra
                                //caso positivo, apenas alterar a produção da regra
                                for(auto& regraVerificacao: gramatica)
                                {   
                                    if(regraVerificacao.second.size() == 1)
                                    {
                                        for(const string &producaoVerificacao : regraVerificacao.second)
                                        {   
                                            if(producaoVerificacao == simbolo)
                                            {
                                                novaProducao += regraVerificacao.first;
                                                achouProducaoIgual = true;
                                            }
                                        }
                                    }
                                }
                                //nenhuma regra que gera essa produção foi encontrada
                                //uma nova regra é criada
                                if(achouProducaoIgual == false)
                                {
                                    string nomeRegra = "R" + to_string(numeroRegra);
                                    gramatica[nomeRegra].push_back(simbolo);
                                    numeroRegra++;
                                    novaProducao += nomeRegra;
                                    criouNovaRegra = true;
                                }
                            }
                            else
                            {
                                novaProducao += simbolo;
                            }
                        }
                        producao = novaProducao;
                    }
                }
            }
        }while(criouNovaRegra == true);
    }
};

int main()
{
    ChomskyNormalForm Gramatica("gramatica.txt");

    if (Gramatica.carregaGramatica())
    {
        Gramatica.adicionaSimboloInicial();
        Gramatica.removeLambda();
        Gramatica.aplicaRegraDaCadeia();
        Gramatica.aplicaTerm();
        Gramatica.aplicaReach();
        Gramatica.aplicaChomsky();
        Gramatica.printaGramatica();
    }
    else
    {
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
