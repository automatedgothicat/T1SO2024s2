#include <Windows.h>
#include <process.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

//assumindo que a matriz ta pronta
//calcula o numero de submatrizes pelo tamanho
//guarda todas elas em um vetor de submatriz
//o vetor tem que estar tendo da thread
//dentro da sessão critica apaga posição do vetor
//outra sessão critica pra somar quando for primo
//quando o vetor acabar encerra thread

using namespace std;

typedef struct {
    int id;
    int count;
    bool stopThread;
}PARAMETROS;

HANDLE hMutex1;
HANDLE hMutex2;
HANDLE hMutex3;
int** matriz;
int qtdprimos;
int linhas, colunas, seed;

void testaParametro(void* PARAMETRO);
bool ehPrimo(int num);
void criarMatriz(int linha, int coluna, int seed);
void destroiMatriz(int linha);
int contagemSerial();

int main()
{
    int nThreads = 4; //2 nucleos reais com 4 virtuais
    int escolhaMenu;
    clock_t ini, fim, total;

    do {
        std::cout <<
            "--------------------------------Menu--------------------------------\n" <<
            "|1) Definir o tamanho da matriz                                    |\n" <<
            "|2) Definir semente para o gerador de numeros aleatorios           |\n" <<
            "|3) Preencher a matriz com numeros aleatorios                      |\n" <<
            //"|4) Definir o tamanho das submatrizes                              |\n" <<
            //"|5) Definir o numero de Threads                                    |\n" <<
            //"|6) Executar                                                       |\n" <<
            "|7) Visualizar o tempo de execucao e quantidade de numeros primos  |\n" <<
            "|8) Encerrar                                                       |\n" <<
            "--------------------------------------------------------------------\n" <<
            "Opcao: ";
        cin >> escolhaMenu;
        switch (escolhaMenu)
        {
        case 1: {
            cout << "Digite o numero de linhas: ";
            cin >> linhas;
            cout << "Digite o numero de colunas: ";
            cin >> colunas;
            cout << "Matriz de " << linhas << " x " << colunas << " definida.\n";
            break;
        }
        case 2: {
            cout << "Digite a semente para o gerador de numeros aleatorios: ";
            cin >> seed;
            cout << "Semente definida: " << seed << endl;
            break;
        }
        case 3: {
            if (linhas > 0 && colunas > 0) {
                criarMatriz(linhas, colunas, seed);
                cout << "Matriz preenchida com numeros aleatorios.\n";
            }
            else {
                cout << "Defina o tamanho da matriz antes de preenche-la.\n";
            }
            break;
        }
        case 7: {
            if (matriz != nullptr) {
                ini = clock();
                qtdprimos = contagemSerial();
                fim = clock();
                total = fim - ini;
                cout << "Quantidade de numeros primos: " << qtdprimos << endl;
                if (total > 1000) {
                    cout << "Tempo de execucao: " << total/1000 << " segundos\n" << endl;
                }
                else {
                    cout << "Tempo de execucao: " << total << " milisegundos\n" << endl;
                }
            }
            else {
                cout << "Preencha a matriz com números aleatorios primeiro.\n";
            }
            break;
        }
        case 8: {
            cout << "Encerrando o programa...\n";
            destroiMatriz(linhas);  
            break;
        }
        default: {
            cout << "Opcao invalida. Tente novamente.\n";
            break;
        }
        }
    } while (escolhaMenu != 8);

    /*
    vector<HANDLE> hThread;
    vector<PARAMETROS> vetorparametros;
    PARAMETROS adicionarVetor;

    hMutex1 = CreateMutex(NULL, FALSE, NULL);

    for (int i = 0; i < nThreads; i++)
    {
        adicionarVetor.id = i;
        adicionarVetor.count = 0;
        adicionarVetor.stopThread = FALSE;
        vetorparametros.push_back(adicionarVetor);
    }

    for (int i = 0; i < nThreads; i++)
    {
        hThread.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&testaParametro, &vetorparametros[i], CREATE_SUSPENDED, NULL));
    }

    for (int i = 0; i < nThreads; i++)
    {
        ResumeThread(hThread[i]);
    }

    WaitForMultipleObjects(nThreads, hThread.data(), TRUE, INFINITE);

    for (int i = 0; i < nThreads; i++)
    {
        CloseHandle(hThread[i]);
    }
    
    */
}

void testaParametro(void* parametroFuncao)
{
    PARAMETROS* param = (PARAMETROS*)parametroFuncao;

    WaitForSingleObject(hMutex1, INFINITE);//inicio seção critica
    cout << "thread id: " << param->id << endl;
    cout << "valor count: " << param->count << endl;
    cout << "stop thread: " << param->stopThread << endl;
    ReleaseMutex(hMutex1); //final seção critica

    _endthread();
}

bool ehPrimo(int num) {
    if (num <= 1) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;
    for (int i = 3; i <= sqrt(num); i += 2) {
        if (num % i == 0) return false;
    }
    return true;
}

void criarMatriz(int linha, int coluna, int seed) {
    srand(seed);

    matriz = new int* [linha];
    for (int i = 0; i < linha; i++)
        matriz[i] = new int[coluna];

    for (int i = 0; i < linha; i++) {
        for (int j = 0; j < coluna; j++) {
            matriz[i][j] = rand() % 100000000;
        }
    }
}

void destroiMatriz(int linha) {
    for (int i = 0; i < linha; i++)
        delete[] matriz[i];
    delete[] matriz;
}

int contagemSerial() {
    int nprimos = 0;
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            if (ehPrimo(matriz[i][j])) {
                nprimos++;
            }
        }
    }
    return nprimos;
}