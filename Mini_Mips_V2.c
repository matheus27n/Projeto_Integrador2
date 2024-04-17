#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_MEMORIA 256 // Tamanho da memória 
#define TAM_REGISTRADORES 8 // Ajustado para incluir o registrador $SW
#define TAM_INSTRUCAO 17 // 16 bits + 1 para o caractere nulo
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
Instrucao codificarInstrucao(char *instrucao_string); // Protótipo adicionado

int main() {
    BancoRegistradores banco_registradores;
    PC pc;
    inicializarBancoRegistradores(&banco_registradores);
    inicializarPC(&pc);
    carregarMemoria(); // Carregar a memória de instrução
    inicializarMemoriaDados();// Inicializar a memória de dados

    int opcao;
    do {
        printf("\nMenu Principal\n"); 
        printf("1. Carregar memoria\n"); 
        printf("2. Imprimir memoria \n"); 
        printf("3. Imprimir registradores \n"); 
        printf("4. Imprimir todo o simulador \n"); 
        printf("5. Salvar .asm \n"); 
        printf("6. Salvar .mem \n"); 
        printf("7. Executa Programa (run)\n"); 
        printf("8. Executa uma instrucao (Step)\n"); 
        printf("9. Volta uma instrucao (Back)\n"); 
        printf("0. Sair \n"); 
        printf("Escolha uma opcao: "); 
        scanf("%d", &opcao);
     
        switch (opcao) {
            case 1:
                carregarMemoria();
                break;
            case 2:
                imprimirMemoria(memoria_instrucao);
                break;
            case 3:
                imprimirRegistradores(&banco_registradores);
                break;
            case 4:
                imprimirRegistradores(&banco_registradores);
                imprimirMemoriaDados();
                break;
            case 7:
                // Implemente a execução do programa
                while (pc.endereco_atual < 12) {
                    executarInstrucao(codificarInstrucao(memoria_instrucao[pc.endereco_atual]), &banco_registradores, &pc);
                }
                imprimirRegistradores(&banco_registradores);
                imprimirMemoriaDados();
                printf("Programa executado com sucesso\n");
                break;
            case 8: 
                // Implemente a execução de uma instrução
                executarInstrucao(codificarInstrucao(memoria_instrucao[pc.endereco_atual]), &banco_registradores, &pc);
                break;
            default:
                printf("Opção inválida! Escolha uma opção válida.\n");
                // Adicione mais casos para as outras opções
        }
    } while (opcao != 0);
    return 0;
}

void inicializarBancoRegistradores(BancoRegistradores *banco_registradores) {
    for(int i = 0; i < TAM_REGISTRADORES; i++) {
        banco_registradores->registradores[i] = 0;
    }
}

void inicializarPC(PC *pc) {
    pc->endereco_atual = 0;
    pc->endereco_proximo = 0;
}

void inicializarMemoriaDados() {
    // Inicializar a memória de dados com zeros
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        memoria_dados[i] = 0;
    }
}

void carregarMemoria() {
    FILE *arquivo_memoria = fopen("C:\\Users\\Matheus\\Desktop\\MINI MIPS 8 BITS\\Mini_mips_v2\\instrucoes.txt", "r");
    if (arquivo_memoria == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    int i = 0; // Começa a partir do endereço 0
    char linha[TAM_INSTRUCAO + 1]; // +1 para o caractere nulo
    while (fgets(linha, TAM_INSTRUCAO + 1, arquivo_memoria)) {
        char *pos; // Ponteiro para a posição do caractere de nova linha
        if ((pos = strchr(linha, '\n')) != NULL) {
            *pos = '\0'; // Substitui o caractere de nova linha por nulo
        }
        strncpy(memoria_instrucao[i], linha, TAM_INSTRUCAO); // Copia a linha para a memória de instrução
        i++;
    }
    fclose(arquivo_memoria);
    printf("Memoria carregada com sucesso\n");
}

void imprimirMemoria(char memoria_instrucao[][TAM_INSTRUCAO]) {
    printf("Conteudo da memoria:\n");
    for (int i = 0; i < /*TAM_MEMORIA_DADOS*/ 12; i++) {
        printf("Endereco %d: %s\n", i, memoria_instrucao[i]);
        // Decodifica e imprime a instrução
        codificarInstrucao(memoria_instrucao[i]);
        printf("\n");
    }
}

void imprimirMemoriaDados() {
    printf("Conteudo da memoria de dados:\n");
    for (int i = 0; i < /*TAM_MEMORIA_DADOS*/ 12; i++) {
        printf("Endereco %d: [%d]\n", i, memoria_dados[i]);
    }
}

void imprimirRegistradores(BancoRegistradores *banco_registradores) {
    printf("Conteudo do banco de registradores:\n");
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, banco_registradores->registradores[i]);
    }
}

void imprimirInstrucao(Instrucao inst) {
    switch (inst.tipo) {
        case R_TYPE:
            printf("Tipo: R_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Rs (Origem): %d\n", inst.rs);
            printf("Rt (Destino): %d\n", inst.rt);
            printf("Rd (Destino): %d\n", inst.rd);
            printf("Funct (Funcao(AND/SUB)): %d\n", inst.funct);
            break;
        case I_TYPE:
            printf("Tipo: I_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Rs (origem): %d\n", inst.rs);
            printf("Rt (origem): %d\n", inst.rt);
            printf("Imediato: %d\n", inst.imm);
            break;
        case J_TYPE:
            printf("Tipo: J_TYPE\n");
            printf("Opcode: %d\n", inst.opcode);
            printf("Addr: %d\n", inst.addr);
            break;
    }
}

int ula(int a, int b, int op) {
    switch (op) {
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
    if (select == 0) {
        return a;
    } else {
        return b;
    }
}

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
            inst.opcode = (instrucao_int >> 12) & 0xF; // 4 bits mais significativos
            inst.rs = (instrucao_int >> 9) & 0x7; // 3 bits seguintes
            inst.rt = (instrucao_int >> 6) & 0x7; // 3 bits seguintes
            inst.rd = (instrucao_int >> 3) & 0x7; // 3 bits seguintes
            inst.funct = instrucao_int & 0x7; // 3 bits menos significativos
            break;
        case I_TYPE:
            inst.opcode = (instrucao_int >> 12) & 0xF; // 4 bits mais significativos
            inst.rs = (instrucao_int >> 9) & 0x7; // 3 bits seguintes
            inst.rt = (instrucao_int >> 6) & 0x7; // 3 bits seguintes
            inst.imm = instrucao_int & 0x3F; // 6 bits menos significativos
            break;
        case J_TYPE:
            inst.opcode = (instrucao_int >> 12) & 0xF; // 4 bits mais significativos
            inst.addr = instrucao_int & 0xFFF; // 12 bits menos significativos
            break;
    }
    imprimirInstrucao(inst); // Chama a função para imprimir os campos da instrução
    return inst;
}

// Implemente as funções para executar o programa e as instruções
void executarInstrucao(Instrucao inst, BancoRegistradores *banco_registradores, PC *pc) {
    
    switch (inst.tipo) {
        case R_TYPE:
            // Execução das instruções do tipo R
            switch (inst.funct) {
                case 0:  // add
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] + banco_registradores->registradores[inst.rt];
                    break;
                case 2: // sub 
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] - banco_registradores->registradores[inst.rt];
                    break;
                case 4: // and
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] & banco_registradores->registradores[inst.rt];
                    break;
                case 5: // or
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] | banco_registradores->registradores[inst.rt];
                    break;
                // Adicione mais casos conforme necessário
                default:
                    printf("Funcao R nao reconhecida: %d\n", inst.funct);
                    break;
            }
            break;
         case I_TYPE:
            // Execução das instruções do tipo I
            switch (inst.opcode) {
                case 8: // addi
                    banco_registradores->registradores[inst.rt] = banco_registradores->registradores[inst.rs] + inst.imm;
                    memoria_dados[inst.rt] = banco_registradores->registradores[inst.rt];// Atualizar a memória de dados
                    break;
                // Outros casos...
            }
            break;
        case J_TYPE:
            // Execução das instruções do tipo J
            printf("Executando instrução do tipo J\n");
            break;
    }

    pc->endereco_atual = pc->endereco_proximo; // Atualizar o endereço atual
    pc->endereco_proximo++; // Avançar para a próxima instrução na sequência
        printf("PC atual: %d\n", pc->endereco_atual);
        printf("PC próximo: %d\n", pc->endereco_proximo);
}