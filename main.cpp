#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class ChomskyNormalForm
{
private:
    // Cria uma tabela hash;
    // Uma string, que é o estado, e um vetor de string, que são as regras
    unordered_map<string, vector<string>> gramatica;
    string filename;
    string simboloInicial;

public:
    ChomskyNormalForm(const string &filename) : filename(filename), simboloInicial("") {}

    bool carregaGramatica()
    { // função pra carregar a gramática do arquivo
        ifstream file(filename);
        if (!file.is_open())
        { // checa erro no processo
            cout << "Erro ao abrir o arquivo!" << endl;
            return false;
        }

        // Loop para ler o arquivo
        // initialSymbol recebe o símbolo inicial da gramática
        // se countInitialAppearance==0, o primeiro símbolo está sendo lido, então ele é o inicial
        string line;
        int countIniciais = 0;
        while (getline(file, line))
        { // lê o arquivo linha por linha
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
            // (inclusive os próximos iniciais) a ela?
        }

        file.close();
        return true;
    }

    // Verifica a gramática em busca de recursividade do símbolo inicial
    bool checaRecursaoInicial()
    {
        // Esse for é executado para cada regra da gramática
        for (const auto &regra : gramatica)
        { // 'regra' é cada par de chave e valor de 'gramatica'
            // Esse for é executado para cada produção da regra
            for (const string &production : regra.second)
            {
                if (production.find(simboloInicial) == 1)
                { // sem esse 1 aqui o if não funciona direito
                    return true;
                }
            }
        }
        return false;
    }

    void adicionaSimboloInicial()
    {
        if (checaRecursaoInicial())
        {
            gramatica["S'"].push_back(simboloInicial);
        }
    }

    // Printa a gramática
    void printaGramatica() const
    { // o-> pra que esse const depois?
        for (const auto &regra : gramatica)
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

    void removeLambda()
    {
        vector<string> producoesLambda;
        // Encontra regras que produzem lambda e já remove o lambda
        for (auto &regra : gramatica)
        {
            if (regra.first != simboloInicial)
            {
                for (size_t i = 0; i < regra.second.size(); i++)
                {
                    if (regra.second[i] == ".")
                    {
                        producoesLambda.push_back(regra.first);
                        gramatica[regra.first].erase(gramatica[regra.first].begin() + i); // !
                        i--;                                                              // Ajusta o índice para não pular produções
                    }
                }
            }
        }

        // Para cada regra de cada estado, verifica todos os caracteres da regra
        // Se a regra possui apenas simbolos anuláveis, o estado é anulável e adicionado no vetor
        // Toda vez que um novo estado anulável é encontrado, o loop reinicia, fazendo uma nova verificação
        bool adicionouNovasAnulaveis;
        do
        {
            adicionouNovasAnulaveis = false;

            for (auto &regra : gramatica)
            {
                // Esse if verifica se o estado analisado já não é anulável
                //'find(producoesLambda.begin(),producoesLambda.end(),regra.first)' faz uma busca em 'producoesLambda' pelo estado, do inicio ao fim
                // Se nada for encontrado, o retorno de 'find' vai ser igual a 'producoesLambda.end()' e o código entra no if
                if (find(producoesLambda.begin(), producoesLambda.end(), regra.first) == producoesLambda.end())
                { // if negativo
                    // para cada produção da regra
                    for (const string &producao : regra.second)
                    {
                        bool isAnulavel = true;
                        // para cada caractere da regra
                        // se for encontrado um unico caractere terminal ou que não seja anulável, essa regra não é anulável
                        // a verificação sobre o caractere ser anulável é igual ao if de cima
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

        // para cada regra da gramatica
        bool criouNovaProducao;

        do
        {
            criouNovaProducao = false;
            for (auto &regra : gramatica)
            {
                // verifica se a regra é anulável
                vector<string> novasProducoes;
                // para cada producao da regra
                for (const string &producao : regra.second)
                {
                    // para cada caractere da produção
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
                            { // muito interessante
                                novasProducoes.push_back(novaProducao);
                                criouNovaProducao = true;
                            }
                            // se a nova produção é vazia e está sendo analisado a regra inicial, deve ser inserido o sinal de lambda
                            else if ((novaProducao.empty()) && (regra.first == simboloInicial))
                            {
                                novaProducao = '.';
                                // verifica se ja nao existe lambda e insere
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
        // loop é executado enquanto novas regras forem criadas
    }

    void aplicaRegraDaCadeia()
    { // aplica a regra da cadeia à gramática
        // regra."first" é o símbolo (A, B, S...), e "second" a sua produção.

        for (auto &regra : gramatica)
        { // vamos iteirar pra cada regra da gramática. o loop vai nomear essas regras V, como em "variável".
            const string &V = regra.first;
            unordered_set<string> alcancaveis; // "alcançáveis" em relação à cadeia, não ao "reach".
            alcancaveis.insert(V);

            bool adicionou; // "novo não-terminal já foi adicionado à alcancaveis?"

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
        } while (encontrouNovoTerminal == true); // pra repetir o processo inteiro, caso encontrado

        // símbolos não terminais removidos.
        // ->
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


    // produces de tamanho 2 ou 1
    // se tamanho 2: 2 nao terminais
    // se tamanho 1: 1 terminal
    void aplicaChomsky()
    {
        // Para cada regra da gramática
        //   Para cada produção da regra
        //       Se a regra possui tamanho > 2:
        //           Mantém primeiro símbolo
        //           Cria um novo estado com uma produção que contém os simbolos restantes
        //           Verifica se esse estado já não existe. Caso não exista, adicione ele a gramática
        //   Verifica a gramática até não gerar mais nenhum estado novo
   
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
                            //criouEstadoNovo=true;
                        }
                    }
                }
            }
        }while(criouEstadoNovo == true);

        // Para cada regra da gramática
        //   Para cada produção da regra
        //       Se a produção possui tamanho == 2 e um não terminal:
        //           Cria um estado com uma produção para o não terminal

        //remover a regra e adicionar uma nova correta

        //verificar se funciona corretamente quando chamar um estado que tem nome de tamanho 2 (R1, R2, R3)
        bool criouNovaRegra = true;
        do{
            criouNovaRegra = false;
            for(auto &regra : gramatica)
            {
                cout << "Verificando a regra " << regra.first << endl;
                for(string &producao : regra.second)
                {   
                    cout << "Verificando a producao " << producao << endl;
                    if(producao.size() == 2){
                        cout << " -> A producao possui tamanho 2 " <<  endl;
                        string novaProducao;
                        for(char &caractere : producao)
                        {
                            cout << "Verificando o caractere " << caractere << endl;
                            if(!isupper(caractere))
                            {   cout << "O caractere " << caractere << " é terminal " << endl;
                                bool modificouProducao = false;
                                bool achouProducaoIgual = false;
                                //verificar se ja existe uma regra que tem uma produdao que produz apenas esse terminal
                                //caso negativo, criar uma nova regra
                                //caso positivo, apenas alterar a produção da regra
                                string stringCaractere(1, caractere);
                                string novoCaractere;
                                for(auto& regraVerificacao: gramatica)
                                {   
                                    if(regraVerificacao.second.size() == 1)
                                    {
                                        for(const string &producaoVerificacao : regraVerificacao.second)
                                        {   
                                            if(producaoVerificacao == stringCaractere){
                                                cout << "A regra " << regraVerificacao.first << " possui uma producao de tamanho 1 que gera " << caractere << endl;
                                                //cout << "Uma produção foi modificada: " << producao << " foi alterada para: ";
                                                novoCaractere += regraVerificacao.first;
                                                modificouProducao = true;
                                                achouProducaoIgual = true;
                                                //cout << producao << endl;
                                            }
                                        }
                                    }
                                }
                                //Não encontrou produção igual
                                //Necessário criar nova regra
                                if(achouProducaoIgual == false){
                                    cout << "O caractere " << caractere << " é terminal e nenhuma produção foi encontrada para ele. Criando nova regra..." << endl;
                                    string nomeRegra = "R" + to_string(numeroRegra);
                                    gramatica[nomeRegra].push_back(stringCaractere);
                                    numeroRegra++;
                                    novaProducao += nomeRegra;
                                    modificouProducao = true;
                                    criouNovaRegra = true;
                                }
                                if(modificouProducao == true){
                                    novaProducao += novoCaractere;
                                }
                            }
                            //Se o caractere é maiúsculo, apenas o copia
                            else
                            {
                                novaProducao += caractere;
                            }
                        }
                        cout << "A producao " << producao << " vai receber um novo valor: " << novaProducao << endl;
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
        // Gramatica.adicionaSimboloInicial();
        // Gramatica.removeLambda();
        // Gramatica.aplicaRegraDaCadeia();
        // Gramatica.aplicaTerm();
        // Gramatica.aplicaReach();
        Gramatica.aplicaChomsky();
        Gramatica.printaGramatica();
    }
    else
    {
        cout << "Erro ao acessar o arquivo";
    }

    return 0;
}
