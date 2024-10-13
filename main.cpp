#include <Windows.h>
#include <process.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

/*
assumindo que a matriz ta pronta
calcula o numero de submatrizes pelo tamanho
guarda todas elas em um vetor de submatriz
o vetor tem que estar dentro da thread
dentro da sessão critica apaga posição do vetor
outra sessão critica pra somar quando for primo
quando o vetor acabar encerra thread
*/

using namespace std;

typedef struct {
    int startline;
    int endline;
    bool escolhida;
}submatriz;

HANDLE hMutex1;
HANDLE hMutex2;
HANDLE hMutex3;
int** matriz;
int qtdprimos,qtdprimosemserie;
int linhas, colunas, seed;
vector<submatriz> vetorsubmatriz;

void calculaMatriz(void* submatrizes);
int checkInt(int num);
bool ehPrimo(int num);
void criarMatriz();
void destroiMatriz();
int contagemSerial();
void setarSubmatrizes(int nthreads);

int main()
{
    int nThreads = 0; //2 nucleos reais com 4 virtuais
    int escolhaMenu;
    clock_t serieIni, serieFim, serieTotal;
    clock_t paraleloIni, paraleloFim, paraleloTotal;
    vector<HANDLE> hThread;
    
    hMutex1 = CreateMutex(NULL, FALSE, NULL);
    hMutex2 = CreateMutex(NULL, FALSE, NULL);

    do {
        std::cout <<
            "x --------------------------- x Menu x --------------------------- x\n" <<
            "1) Definir o tamanho da matriz                                    \n" <<
            "2) Definir semente para o gerador de numeros aleatorios           \n" <<
            "3) Preencher a matriz com numeros aleatorios                      \n" <<
            "4) Definir o tamanho das submatrizes                              \n" <<
            "5) Definir o numero de Threads                                    \n" <<
            "6) Executar                                                       \n" <<
            "7) Visualizar o tempo de execucao e quantidade de numeros primos  \n" <<
            "8) Encerrar                                                       \n" <<
            "Opcao: ";
        cin >> escolhaMenu;
        switch (escolhaMenu)
        {
        case 1: {
            cout << "Digite o numero de linhas: ";
            cin >> linhas;
            linhas = checkInt(linhas);
            cout << "Digite o numero de colunas: ";
            cin >> colunas;
            colunas = checkInt(colunas);
            cout << "Matriz de " << linhas << " x " << colunas << " definida.\n";
            break;
        }
        case 2: {
            cout << "Digite a semente para o gerador de numeros aleatorios: ";
            cin >> seed;
            seed = checkInt(seed);
            cout << "Semente definida: " << seed << endl;
            break;
        }
        case 3: {
            if (linhas > 0 && colunas > 0) {
                criarMatriz();
                cout << "Matriz preenchida com numeros aleatorios.\n";
            }
            else {
                cout << "Defina o tamanho da matriz antes de preenche-la.\n";
            }
            break;
        }
        case 4: {
            if (nThreads == 0) {
                cout << "Primeiro Defina as Threads\n" << endl;
                break;
            }
            cout << "Matriz dividida em " << nThreads << " submatrizes\n" << endl;
            setarSubmatrizes(nThreads);
            break;
        }
        case 5: {
            if (nThreads != 0) {
                for (int i = 0; i < nThreads; i++) {
                    CloseHandle(hThread[i]);
                }
                for (int i = 0; i < nThreads; i++) {
                    hThread.pop_back();
                    vetorsubmatriz.pop_back();
                }
            }
            cout << "Digite o numero de threads a ser utilizado: ";
            cin >> nThreads;
            seed = checkInt(nThreads);
            cout << "Threads criadas: " << nThreads << endl;
            break;
        }
        case 6: {
            if (linhas <= 0 && colunas <= 0) {
                cout << "Primeiro gere a matriz" << endl;
                break;
            }
            if (nThreads == 0) {
                cout << "Primeiro Defina as Threads" << endl;
                break;
            }
            serieIni = clock();
            qtdprimosemserie = contagemSerial();
            serieFim = clock();
            serieTotal = serieFim - serieIni;

            //configura as threads com a função passada de argumento e o endereço do vetor usado
            for (int i = 0; i < nThreads; i++)
            {
                hThread.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&calculaMatriz, &vetorsubmatriz[i], CREATE_SUSPENDED, NULL));
            }

            //inicializa cada thread no vetor hthreads que estavam suspensas
            paraleloIni = clock();
            for (int i = 0; i < nThreads; i++)
            {
                ResumeThread(hThread[i]);
            }

            //aguarda que todas as threads executem corretamente
            WaitForMultipleObjects(nThreads, hThread.data(), TRUE, INFINITE);
            paraleloFim = clock();
            paraleloTotal = paraleloFim - paraleloIni;

            //finaliza cada thread no vetor de threads
            for (int i = 0; i < nThreads; i++)
            {
                CloseHandle(hThread[i]);
            }

            CloseHandle(hMutex1);
            CloseHandle(hMutex2);

            break;
        }
        case 7: {
            if (matriz != nullptr) {
                cout << "Contagem em serie:" << endl;
                cout << "Quantidade de numeros primos: " << qtdprimosemserie << endl;
                if (serieTotal > 1000) {
                    cout << "Tempo de execucao: " << serieTotal / 1000 << " segundos\n" << endl;
                }
                else {
                    cout << "Tempo de execucao: " << serieTotal << " milisegundos\n" << endl;
                }
                cout << "Contagem em paralelo:" << endl;
                cout << "Quantidade de numeros primos: " << qtdprimos << endl;
                if (paraleloTotal > 1000) {
                    cout << "Tempo de execucao: " << paraleloTotal / 1000 << " segundos\n" << endl;
                }
                else {
                    cout << "Tempo de execucao: " << paraleloTotal << " milisegundos\n" << endl;
                }
            }
            else {
                cout << "Preencha a matriz com números aleatorios primeiro.\n";
            }
            break;
        }
        case 8: {
            cout << "Encerrando o programa...\n";
            destroiMatriz();  
            break;
        }
        default: {
            cout << "Opcao invalida. Tente novamente.\n";
            break;
        }
        }
    } while (escolhaMenu != 8);
}

int checkInt(int num) {
    while (num <= 0) {
        cout << "Digite um valor maior que 0: ";
        cin >> num;
    }
    return num;
}

void calculaMatriz(void* submatrizes)
{
    //cria um ponteiro pra parametro que recebe os argumentos da função
    submatriz* subdathread = (submatriz*)submatrizes;
    int primosLocais = 0;

    for (int i = subdathread->startline; i < subdathread->endline; i++) {
        for (int j = 0; j < colunas; j++) {
            if (ehPrimo(matriz[i][j])) {
                primosLocais++; 
            }
        }
    }

    WaitForSingleObject(hMutex1, INFINITE);//inicio seção critica
    vetorsubmatriz.pop_back();
    ReleaseMutex(hMutex1); //final seção critica

    WaitForSingleObject(hMutex2, INFINITE);//inicio seção critica
    qtdprimos += primosLocais;
    ReleaseMutex(hMutex2); //final seção critica

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

void criarMatriz() {
    srand(seed);

    matriz = new int* [linhas];
    for (int i = 0; i < linhas; i++)
        matriz[i] = new int[colunas];

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            matriz[i][j] = rand() % 100000000;
        }
    }
}

void destroiMatriz() {
    for (int i = 0; i < linhas; i++)
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

void setarSubmatrizes(int nthreads) {
    int linhaspthread = linhas / nthreads;
    int restolinhas = linhas % nthreads;
    submatriz adicionarSubmatriz;

    int linhaAtual = 0;
    for (int i = 0; i < nthreads; i++) {
        int linhaInicio = linhaAtual;
        int linhaFim = linhaInicio + linhaspthread;

        if (i < restolinhas) {
            linhaFim++;
        }

        adicionarSubmatriz.startline = linhaInicio;
        adicionarSubmatriz.endline = linhaFim;
        adicionarSubmatriz.escolhida = false;
        vetorsubmatriz.push_back(adicionarSubmatriz);

        cout << "O tamanho da submatriz" << i << " e: " << adicionarSubmatriz.endline - adicionarSubmatriz.startline << " x " << colunas << endl;

        linhaAtual = linhaFim;
    }
}