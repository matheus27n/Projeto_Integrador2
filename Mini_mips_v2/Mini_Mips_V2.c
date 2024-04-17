#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_MEMORIA 256 // Tamanho da memória 
#define TAM_REGISTRADORES 8 // Ajustado para incluir o registrador $SW
#define TAM_INSTRUCAO 16 // Tamanho da instrução MIPS
#define TAM_MEMORIA_DADOS 256 // Tamanho da memória de dados

char memoria_instrucao[TAM_MEMORIA][TAM_INSTRUCAO]; // Matriz de caracteres para armazenar as instruções
int memoria_dados[TAM_MEMORIA_DADOS]; // Vetor de inteiros para armazenar os dados

typedef enum { // Enumeração para os tipos de instrução
    R_TYPE,
    I_TYPE,
    J_TYPE
} InstrucaoTipo;

typedef struct { // Estrutura para representar uma instrução MIPS
    InstrucaoTipo tipo;
    char inst_char[TAM_INSTRUCAO]; // 16 caracteres + 1 para o caractere nulo
    int opcode;
    int rs;
    int rt;
    int rd;
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
void inicializarBancoRegistradores();
void carregarMemoria();
void imprimirMemoria();
void imprimirRegistradores();
void imprimirInstrucao(Instrucao inst);
void imprimirMemoriaDados();
int ula(int a, int b, int op);
int mux(int a, int b, int select);
PC inicializarPC();
Instrucao codificarInstrucao(char *instrucao_string);
PC pc; // Contador de programa (PC)
BancoRegistradores banco_registradores; // Banco de registradores

int main() {
    int opcao;

    inicializarBancoRegistradores(); // Inicializa o banco de registradores
    pc = inicializarPC(); // Inicializa o PC

    do {
        printf("\nMenu Principal\n"); 
        printf("1. Carregar memória\n"); 
        printf("2. Imprimir memória \n"); 
        printf("3. Imprimir registradores \n"); 
        printf("4. Imprimir todo o simulador \n"); 
        printf("5. Salvar .asm \n"); 
        printf("6. Salvar .mem \n"); 
        printf("7. Executa Programa (run)\n"); 
        printf("8. Executa uma instrução (Step)\n"); 
        printf("9. Volta uma instrução (Back)\n"); 
        printf("0. Sair \n"); 
        printf("Escolha uma opção: "); 
        scanf("%d", &opcao);
     
        switch (opcao) {
            case 1:
                carregarMemoria();
                break;
            case 2:
                imprimirMemoria();
                break;
            case 3:
                imprimirRegistradores();
                break;
            case 4:
                imprimirRegistradores();
                imprimirMemoriaDados();
                break;
            case 7:
                break;
            case 8: 
                break;
            default:
                printf("Opção inválida! Escolha uma opção válida.\n");
                // Adicione mais casos para as outras opções
        }
    } while (opcao != 0);
    return 0;
}

void inicializarBancoRegistradores() {
    for(int i = 0; i < TAM_REGISTRADORES; i++) {
        banco_registradores.registradores[i] = 0;
    }
}

PC inicializarPC() {
    PC pc;
    pc.endereco_atual = 0;
    pc.endereco_proximo = 0;
    return pc;
}

//CARREGAR MEMÓRIA
void carregarMemoria(){
    FILE *arquivo_memoria;
    printf("Informe o nome do arquivo: ");
    char nome_arquivo[50];
    scanf("%s", nome_arquivo);
    arquivo_memoria = fopen(nome_arquivo, "r");
    if(arquivo_memoria == NULL){
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    int i = 0; // Começa a partir do endereço 1
    char linha[TAM_INSTRUCAO];
    while(fgets(linha, TAM_INSTRUCAO, arquivo_memoria )){ //enquanto houver linhas no arquivo, leia
        char *pos; // Ponteiro para a posição do caractere de nova linha
        if((pos = strchr(linha, '\n')) != NULL){ // Verifica se o caractere de nova linha está presente
            *pos = '\0'; // Substitui o caractere de nova linha por nulo
        }
        strncpy(memoria_instrucao[i], linha, TAM_INSTRUCAO); // Copia a linha para a memória de instrução
        i++;
    }
    fclose(arquivo_memoria);
    printf("Memoria carregada com sucesso\n");
}

//IMPRIMIR MEMÓRIA
void imprimirMemoria(){
    printf("Conteudo da memoria:\n");
    for(int i = 1; i < 12; i++){
        printf("Endereco %d: %s\n", i, memoria_instrucao[i]);
        // Decodifica e imprime a instrução
        codificarInstrucao(memoria_instrucao[i]);
        printf("\n");
    }
}

void imprimirMemoriaDados() {
    printf("Conteúdo da memória de dados:\n");
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        printf("Endereco %d: [%d]\n", i, memoria_dados[i]);
    }
}
//IMPRIMIR REGISTRADORES
void imprimirRegistradores() {
    printf("Conteúdo do banco de registradores:\n");
    for(int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, banco_registradores.registradores[i]);
    }
}

// Imprimir os campos da instrução com base no tipo
void imprimirInstrucao(Instrucao inst) {
    switch (inst.tipo) {
        case R_TYPE:
            printf("Tipo: R_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Rs: %d\n", inst.rs);
            printf("Rt: %d\n", inst.rt);
            printf("Rd: %d\n", inst.rd);
            printf("Funct: %d\n", inst.funct);
            break;
        case I_TYPE:
            printf("Tipo: I_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Rs: %d\n", inst.rs);
            printf("Rt: %d\n", inst.rt);
            printf("Imm: %d\n", inst.imm);
            break;
        case J_TYPE:
            printf("Tipo: J_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Addr: %d\n", inst.addr);
            break;
    }
}

// Funções da ULA e MUX
int ula(int a, int b, int op) {
    switch(op) {
        case 0: // ADD
            return a + b;
        case 1: // SUB
            return a - b;
        case 2: // AND
            return a & b;
        case 3: // OR
            return a | b;
        // Adicione mais operações da ULA conforme necessário
        default:
            printf("Operação da ULA não reconhecida: %d\n", op);
            return 0;
    }
}


int mux(int a, int b, int select) {
    if(select == 0) {
        return a;
    } else {
        return b;
    }
}



// Codificar a instrução MIPS
Instrucao codificarInstrucao(char *instrucao_string) {
    Instrucao inst;
    
    unsigned int instrucao_int = strtol(instrucao_string, NULL, 2); //converte para inteiro
    
    // Determina o tipo de instrução com base no opcode
    int opcode = (instrucao_int >> 12) & 0xF; //intrucao_int >> 12 significa que estamos deslocando 12 bits para a direita
    if (opcode == 0) {
        inst.tipo = R_TYPE;
    } else if (opcode == 2 || opcode == 3) {
        inst.tipo = J_TYPE;
    } else {
        inst.tipo = I_TYPE;
    }

    // Extrai os campos da instrução com base nos tipos R, I e J
    switch (inst.tipo) {
        case R_TYPE:
            inst.opcode = (instrucao_int >> 12) & 0xF; //0xF representa os 4 bits mais significativos
            inst.rs = (instrucao_int >> 9) & 0x7; //0x7 representa os 3 bits mais significativos
            inst.rt = (instrucao_int >> 6) & 0x7; //0x7 representa os 3 bits mais significativos
            inst.rd = (instrucao_int >> 3) & 0x7; //0x7 representa os 3 bits mais significativos
            inst.funct = instrucao_int & 0x7; //0x7 representa os 3 bits mais significativos
            break;
        case I_TYPE:
            inst.opcode = (instrucao_int >> 12) & 0xF;
            inst.rs = (instrucao_int >> 9) & 0x7;
            inst.rt = (instrucao_int >> 6) & 0x7;
            inst.imm = instrucao_int & 0x3F; // Valor imediato de 6 bits
            break;
        case J_TYPE:
            inst.opcode = (instrucao_int >> 12) & 0xF;
            inst.addr = instrucao_int & 0xFFF; // Endereço de 12 bits
            break;
    }

    imprimirInstrucao(inst); // Chama a função para imprimir os campos da instrução

    return inst;
}
