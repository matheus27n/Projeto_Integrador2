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

struct nodo{
	BancoRegistradores banco_undo;
	PC pc_undo;
	int mem_dados_undo[TAM_MEMORIA_DADOS];;
};

// Protótipos das funções
int menu();
void inicializarBancoRegistradores(BancoRegistradores *banco_registradores);
void inicializarPC(PC *pc);
void inicializarMemoriaDados(); // Adicionado protótipo
void carregarMemoria();
void carregarMemoriaDados();
void imprimirMemoria(char memoria_instrucao[][TAM_INSTRUCAO]);
void imprimirRegistradores(BancoRegistradores *banco_registradores);
void imprimirInstrucao(Instrucao inst);
void imprimirMemoriaDados();
int ula(int a, int b, int op);
int mux(int a, int b, int select);
void executarPipeline();
void executarTodoPipeline();
void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm, Instrucao inst);
void salvar_data();
void salvar_asm();
Instrucao codificarInstrucao(char *instrucao_string); // Protótipo adicionado
int check_overflow(int result);
int sign_extend(int value, int original_bits);
void undo(struct nodo *backup, PC *pc, int *memoria_dados, BancoRegistradores *banco_registradores);
struct nodo* save_backup(PC*pc, int memoria_dados[], BancoRegistradores *banco_registradores);





//PIPELINE
// Definindo os registros entre estágios do pipeline
typedef struct {
    Instrucao instrucao;
    int pc;
} Registrador_IF_ID;

typedef struct {
    Instrucao instrucao;
    int pc;
} Registrador_ID_EX;

typedef struct {
    Instrucao instrucao; // Alterado para armazenar a instrução completa
    int pc;
    int resultado;
    int reg_rt;
    int reg_rd;
    int mem_read;
    int mem_write;
    int reg_write;
} Registrador_EX_MEM;

typedef struct {
    Instrucao instrucao; // Alterado para armazenar a instrução completa
    int pc;
    int mem_data;
    int resultado;
    int reg_rd;
    int reg_write;
} Registrador_MEM_WB;


void estagio_IF(PC *pc, Registrador_IF_ID *if_id, Instrucao *instrucao);
void estagio_ID(Registrador_IF_ID *if_id, Registrador_ID_EX *id_ex, BancoRegistradores *banco_registradores, Instrucao *instrucao);
void estagio_EX(Registrador_ID_EX *id_ex, Registrador_EX_MEM *ex_mem, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores, Instrucao *instrucao);
void estagio_MEM(Registrador_EX_MEM *ex_mem, Registrador_MEM_WB *mem_w, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores);
void estagio_WB(Registrador_MEM_WB *mem_wb, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores);