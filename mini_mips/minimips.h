#include <stdio.h>

#define TAM_MEMORIA 256 // Tamanho da memória 
#define TAM_REGISTRADORES 8 // Ajustado para incluir o registrador $SW
#define TAM_INSTRUCAO 17 // 16 bits + 1 para o caractere nulo
#define TAM_MEMORIA_DADOS 256 // Tamanho da memória de dados

extern char memoria_instrucao[TAM_MEMORIA][TAM_INSTRUCAO]; // Declaração externa das variáveis
extern int memoria_dados[TAM_MEMORIA_DADOS]; // Declaração externa das variáveis


typedef enum { // Enumeração para os tipos de instrução
    R_TYPE,
    I_TYPE,
    J_TYPE
} InstrucaoTipo;

typedef struct { // Estrutura para representar uma instrução MIPS
    InstrucaoTipo tipo;
    char inst_char[TAM_INSTRUCAO]; // 16 caracteres + 1 para o caractere nulo
    int opcode;
    int rs; // SOURCE
    int rt; // TARGET
    int rd; // DESTINATION
    int funct;
    int imm;
    int addr;
} Instrucao;

typedef struct { // Estrutura para representar o banco de registradores
    int registradores[TAM_REGISTRADORES];
} BancoRegistradores;

typedef struct { // Estrutura para representar o contador de programa (PC)
    int endereco_atual; // Endereço da instrução atual
    int endereco_proximo; // Endereço da próxima instrução a ser executada
} PC;

// Protótipos das funções
int menu();
void inicializarBancoRegistradores(BancoRegistradores *banco_registradores);
void inicializarPC(PC *pc);
void inicializarMemoriaDados(); // Adicionado protótipo
void carregarMemoria();
void imprimirMemoria(char memoria_instrucao[][TAM_INSTRUCAO]);
void imprimirRegistradores(BancoRegistradores *banco_registradores);
void imprimirInstrucao(Instrucao inst);
void imprimirMemoriaDados();
int ula(int a, int b, int op);
int mux(int a, int b, int select);
void executarInstrucao(Instrucao inst, BancoRegistradores *banco_registradores, PC *pc); // Protótipo adicionado
void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm, Instrucao inst);
void salvar_mem();
void salvar_asm();
Instrucao codificarInstrucao(char *instrucao_string); // Protótipo adicionado
