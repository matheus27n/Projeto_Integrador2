#include <stdio.h>

#define TAM_MEMORIA 256 // Tamanho da memória 
#define TAM_REGISTRADORES 8 // Ajustado para incluir o registrador $SW
#define TAM_INSTRUCAO 17 // 16 bits + 1 para o caractere nulo
#define TAM_MEMORIA_DADOS 128 // Tamanho da memória de dados
#define TAM_MEMORIA_INSTRUCAO 128 // Tamanho da memória de instruções

extern char memoria_instrucao[TAM_MEMORIA_INSTRUCAO][TAM_INSTRUCAO]; // Declaração externa das variáveis
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

typedef struct {
    int registradorA;
    int registradorB;
    int registradorSaidaALU;
}RegistradoresEstado;

typedef struct { // Estrutura para representar o contador de programa (PC)
    int endereco_atual; // Endereço da instrução atual
    int endereco_proximo; // Endereço da próxima instrução a ser executada
    //EstadoCiclo estadoCiclo; // Estado atual do ciclo de execução
} PC;

typedef struct { // Estrutura para representar a memória única
    union {
        char instrucoes[TAM_MEMORIA][TAM_INSTRUCAO];
        int dados[TAM_MEMORIA];
    } memoria;
} MemoriaUnica;

struct nodo {
    BancoRegistradores banco_undo;
    PC pc_undo;
    int mem_dados_undo[TAM_MEMORIA_DADOS];
};

// Protótipos das funções
int menu();
void inicializarBancoRegistradores(BancoRegistradores *banco_registradores);
void inicializarPC(PC *pc);
void inicializarMemoriaDados();
void imprimirRegistradores(BancoRegistradores *banco_registradores);
void imprimirInstrucao(Instrucao inst);
int ula(int a, int b, int op);
int mux(int a, int b, int select);
void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm, Instrucao inst);
void salvar_data();
void salvar_asm();
Instrucao codificarInstrucao(char *instrucao_string);
int check_overflow(int result);
int sign_extend(int value, int original_bits);
void undo(struct nodo *backup, PC *pc, int *memoria_dados, BancoRegistradores *banco_registradores);
struct nodo* save_backup(PC*pc, int memoria_dados[], BancoRegistradores *banco_registradores);

// Novos protótipos de funções multiciclo
void executarCicloInstrucao(PC *pc, BancoRegistradores *banco_registradores, RegistradoresEstado *registradores_estado);
void carregarMemoriaUnica();
void imprimirMemoriaUnica();
