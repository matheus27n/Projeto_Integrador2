#include <stdio.h>

#define REG_SIZE 7   // Quantidade de registradores

// Estrutura para representar a CPU
typedef struct{
    int pc;                    // Contador de programa
    unsigned char registers[REG_SIZE]; // Registradores
    unsigned char memory[256][15];    // Memória
} CPU;

// Definindo uma enumeração para os tipos de instrução
typedef enum {
    R_TYPE,
    I_TYPE,
    J_TYPE
} InstrType;

// Definindo uma estrutura para representar uma instrução MIPS
typedef struct {
    unsigned char opcode;// Opcode da instrução
    unsigned char rs;    // Registrador de origem (source)
    unsigned char rt;    // Registrador de destino (target)
    unsigned char rd;    // Registrador de destino (dest)
    unsigned short imm;  // Valor imediato (para instruções I-type)
    unsigned short addr; // Endereço de destino (para instruções J-type)
} Instruction;

// Funções para carregar memória e imprimir a memória
void carregar_memoria(CPU *cpu);
void imprimir_memoria(CPU *cpu);
void imprimir_registradores(CPU *cpu);

int main() {
    CPU cpu;
    cpu.pc = 0;

    // Inicializar registradores
    for (int i = 0; i < REG_SIZE; i++) {
        cpu.registers[i] = 0;
    }

    int opcao;

    do {
        printf("\nMenu Principal\n");
        printf("1. Carregar memória\n");
        printf("2. Imprimir memória\n");
        printf("3. Imprimir registradores\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                carregar_memoria(&cpu);
                break;
            case 2:
                imprimir_memoria(&cpu);
                break;
            case 3:
                imprimir_registradores(&cpu);
                break;
            case 0:
                printf("Saindo do programa...\n");
                break;
            default:
                printf("Opção inválida! Escolha uma opção válida.\n");
        }
    } while (opcao != 0);

    return 0;
}

void carregar_memoria(CPU *cpu) {
    FILE *arquivo_mem = fopen("\\Users\\Matheus\\Desktop\\MINI MIPS 8 BITS\\programaTestaInstrucoes2.txt", "r");
    if (arquivo_mem == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 15; j++) {
            fscanf(arquivo_mem, "%d", &cpu->memory[i][j]);
        }
    }
    fclose(arquivo_mem);

    printf("Memória carregada com sucesso!\n");
}

void imprimir_memoria(CPU *cpu) {
    printf("Conteúdo da memória:\n");
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 15; j++) {
            printf("%d ", cpu->memory[i][j]);
        }
        printf("\n");
    }
}

void imprimir_registradores(CPU *cpu) {
    printf("Registradores:\n");
    for (int i = 0; i < REG_SIZE; i++) {
        printf("R%d: %d\n", i, cpu->registers[i]);
    }
}
