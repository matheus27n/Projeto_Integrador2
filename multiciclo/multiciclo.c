#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "multiciclo.h"


// FUNÇÕES GERAIS

char memoria_instrucao[TAM_MEMORIA_INSTRUCAO][TAM_INSTRUCAO]; // Definição das variáveis
int memoria_dados[TAM_MEMORIA_DADOS]; // Definição das variáveis

int menu(){
    int m;
    printf("\n================================\n");
    printf("\t MINI-MIPS 8 BITS - UNIPAMPA\n"); 
    printf("\n================================\n\n");
    printf("1. Carregar memoriaUnica\n"); 
    printf("2. Imprimir memoriaUnica \n");
    printf("3. Imprimir registradores \n"); 
    printf("4. Imprimir todo o simulador \n"); 
    printf("5. Decodificar instrucao atual \n");
    printf("6. Salvar .asm \n"); 
    printf("7. Salvar .data \n"); 
    printf("8. Executa Programa (run)\n"); 
    printf("9. Executa uma instrucao (Step)\n"); 
    printf("10. Volta uma instrucao (Back)\n"); 
    printf("0. Sair \n"); 
    printf("\n================================\n");
    printf("Escolha uma opcao: "); 
    setbuf(stdin,NULL);
    scanf("%d", &m);
    printf("================================\n");
    return m;
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
        default:
            printf("Operacao da ULA nao reconhecida: %d\n", op);
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


Instrucao codificarInstrucao(char *instrucao_string){
    Instrucao inst;
    unsigned int instrucao_int = strtol(instrucao_string, NULL, 2); //converte a string binaria para inteiro
    // Determina o tipo de instrução com base no opcode
    int opcode = (instrucao_int >> 12) & 0xF; //intrucao_int >> 12 significa que estamos deslocando 12 bits para a direita e & 0xF significa que estamos pegando os 4 bits mais significativos
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
    //imprimirInstrucao(inst); // Chama a função para imprimir os campos da instrução
    return inst;
}


// Conversor de instrução binária em assembly
void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm, Instrucao inst) {
    if(inst.rd == 0 && inst.rt == 0 && inst.rs == 0){
        return;
    }
    switch (inst.tipo) {
        case R_TYPE:
            switch (inst.funct) {
                case 0:
                    fprintf(arquivo_asm, "add $r%d, $r%d, $r%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 2:
                    fprintf(arquivo_asm, "sub $r%d, $r%d, $r%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 4:
                    fprintf(arquivo_asm, "and $r%d, $r%d, $r%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 5:
                    fprintf(arquivo_asm, "or $r%d, $r%d, $r%d", inst.rd, inst.rs, inst.rt);
                    break;
                default:
                    fprintf(arquivo_asm, "Funcao R nao reconhecida: %d", inst.funct);
                    break;
            }
            break;
        case I_TYPE:
            switch (inst.opcode) {
                case 4:
                    fprintf(arquivo_asm, "addi $r%d, $r%d, %d", inst.rt, inst.rs, inst.imm);
                    break;
                case 11:
                    fprintf(arquivo_asm, "lw $r%d, %d(R%d)", inst.rt, inst.imm, inst.rs);
                    break;
                case 15:
                    fprintf(arquivo_asm, "sw $r%d, %d($r%d)", inst.rt, inst.imm, inst.rs);
                    break;
                case 8:
                    fprintf(arquivo_asm, "beq $r%d, $r%d, %d", inst.rt, inst.rs, inst.imm);
                    break;
                default:
                    fprintf(arquivo_asm, "Opcode I nao reconhecido: %d", inst.opcode);
                    break;
            }
            break;
        case J_TYPE:
            fprintf(arquivo_asm, "j %d", inst.addr);
            break;
    }

}

int check_overflow(int result) {
    if (result < -128 || result > 127) {
        return 1; // Indica que houve overflow
    }
    return 0; // Sem overflow
}


int sign_extend(int value, int original_bits) {
    int shift = 32 - original_bits;
    return (value << shift) >> shift;
}

// Implemente as funções para executar o programa e as instruções
//executar programa com todos os ciclos necessarios FETCH, DECODE, EXECUTE, MEMORY, WRITEBACK
void executarCicloInstrucao(PC *pc, BancoRegistradores *banco_registradores, RegistradoresEstado *registradores_estado) {
    static int estado = 0; // Para acompanhar o ciclo atual
    static Instrucao instrucao; // Armazenar a instrução entre os ciclos
    int rs_teste, rt_teste, resultado;

    switch (estado) {
        case 0: // ETAPA DE BUSCA DA INSTRUÇÃO
            printf("--------Executando ciclo FETCH-----------\n");
            if (memoria_instrucao[pc->endereco_atual] == NULL || strlen(memoria_instrucao[pc->endereco_atual]) == 0) { // Verifica se a instrução é nula ou vazia
                pc->endereco_atual = pc->endereco_proximo;
                pc->endereco_proximo = pc->endereco_atual + 1;
                estado = 0; // Reinicia o ciclo
                break;
            }
            printf("Instrução buscada: %s\n", memoria_instrucao[pc->endereco_atual]);
            // RI = Mem[PC]
            instrucao = codificarInstrucao(memoria_instrucao[pc->endereco_atual]);
            // PC = PC + 1
            pc->endereco_atual = pc->endereco_proximo;
            pc->endereco_proximo = pc->endereco_atual + 1;
            estado = 1;
            break;

        case 1: // ETAPA DE DECODIFICAÇÃO DA INSTRUCAO E BUSCA DOS REGISTRADORES
            printf("--------Executando ciclo DECODE-----------\n");
            printf("\n");
            // Decodificação da instrução
            switch (instrucao.tipo) {
                case R_TYPE:
                    printf("|--------------------------------|\n");
                    printf("|Instrução do tipo R identificada|\n");
                    printf("|--------------------------------|\n");
                    imprimirInstrucao(instrucao); // Chama a função para imprimir os campos da instrução
                    printf("Instrução do tipo R identificada\n");
                    registradores_estado->registradorA = banco_registradores->registradores[instrucao.rs];
                    registradores_estado->registradorB = banco_registradores->registradores[instrucao.rt];
                    registradores_estado->registradorSaidaALU = 0;
                    printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
                    printf("Registrador A: %d\n", registradores_estado->registradorA);
                    printf("Registrador B: %d\n", registradores_estado->registradorB);
                    estado = 7;
                    break;
                case I_TYPE:
                    printf("|--------------------------------|\n");
                    printf("|Instrução do tipo I identificada|\n");
                    printf("|--------------------------------|\n");
                    imprimirInstrucao(instrucao);
                    registradores_estado->registradorA = banco_registradores->registradores[instrucao.rs];
                    registradores_estado->registradorB = banco_registradores->registradores[instrucao.rt];
                    registradores_estado->registradorSaidaALU = 0;
                    printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
                    printf("Registrador A: %d\n", registradores_estado->registradorA);
                    printf("Registrador B: %d\n", registradores_estado->registradorB);
                    estado = 2;
                    break;
                case J_TYPE:
                    printf("|--------------------------------|\n");
                    printf("|Instrução do tipo J identificada|\n");
                    printf("|--------------------------------|\n");
                    imprimirInstrucao(instrucao);
                    estado = 10;
                    break;
                default:
                    printf("Tipo de instrução desconhecido\n");
                    estado = 0;
                    break;
            }
            break;

        case 2: // EXECUTE (TIPO I)
            printf("--------Executando ciclo EXECUTE (TIPO I)-----------\n");
            switch (instrucao.opcode) {
                case 4: // ADDI
                    printf("Instrução ADDI\n");
                    registradores_estado->registradorSaidaALU = registradores_estado->registradorA + instrucao.imm;
                    printf("executando...\n");
                    estado = 6; // Vai para o ciclo de WRITEBACK
                    break;
                case 11: // LW
                    printf("----------Instrução LW--------------\n");
                    registradores_estado->registradorSaidaALU = registradores_estado->registradorA + instrucao.imm;
                    printf("executando...\n");
                    printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
                    printf("Registrador A: %d\n", registradores_estado->registradorA);
                    printf("Valor do imediato: %d\n", instrucao.imm);
                    estado = 3;
                    break;
                case 15: // SW
                    printf("Instrução SW - EXECUTE SW\n");
                    registradores_estado->registradorSaidaALU = registradores_estado->registradorA + instrucao.imm;
                    printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
                    printf("Registrador A: %d\n", registradores_estado->registradorA);
                    printf("Registrador B: %d\n", registradores_estado->registradorB);
                    estado = 5;
                    break;
                case 8: // BEQ
                    printf("Instrução BEQ\n");
                    rs_teste = banco_registradores->registradores[instrucao.rs];
                    rt_teste = banco_registradores->registradores[instrucao.rt];
                    if (rs_teste == rt_teste) {
                        pc->endereco_proximo = pc->endereco_atual + instrucao.imm;
                        printf("BEQ tomado. Novo endereço próximo: %d\n", pc->endereco_proximo);
                    } else {
                        printf("BEQ não tomado. Endereço próximo: %d\n", pc->endereco_proximo);
                    }
                    estado = 9;
                    break;
                default:
                    printf("Opcode desconhecido\n");
                    estado = 0;
                    break;
            }
            break;

        case 3: // MEMORY LW
            printf("--------Executando ciclo MEMORY LW-----------\n");
            // LW: RDM = Mem[ALUout]
            printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
            printf("Registrador A: %d\n", registradores_estado->registradorA);
            printf("Registrador B: %d\n", registradores_estado->registradorB);
            estado = 4;
            break;

        case 4: // WRITEBACK (LW)
            printf("--------Executando ciclo WRITEBACK LW-----------\n");
            // Escreve o resultado de uma instrução LW no registrador
            banco_registradores->registradores[instrucao.rt] = registradores_estado->registradorSaidaALU;
            printf("valor adicionado ao registrador: %d\n", banco_registradores->registradores[instrucao.rt]);
            printf("Escreveu no registrador com sucesso!!\n");
            estado = 0;
            break;

        case 5: // WRITEBACK (SW)
            printf("--------Executando ciclo WRITEBACK SW-----------\n");
            // Escrever resultados de volta à memória (SW)
            // SW: Mem[ALUout] = B
            memoria_dados[registradores_estado->registradorSaidaALU] = registradores_estado->registradorB;
            estado = 0;
            break;

        case 6: // WRITEBACK (ADDI)
            printf("--------Executando ciclo WRITEBACK-----------\n");
            // Escrever resultados de volta aos registradores (ADDI)
            // ADDI: reg[RI] = ULAout
            banco_registradores->registradores[instrucao.rt] = registradores_estado->registradorSaidaALU;
            printf("Escreveu no registrador com sucesso!!\n");
            estado = 0;
            break;

        case 7: // Estado do tipo R
            printf("-------- Executando ciclo EXECUTE (TIPO R)-----------\n");
            switch (instrucao.funct) {
                case 0: // ADD
                    printf("Instrução ADD\n");
                    registradores_estado->registradorSaidaALU = ula(registradores_estado->registradorA, registradores_estado->registradorB, 0);
                    printf("Registrador de saída da ALU: %d\n", registradores_estado->registradorSaidaALU);
                    estado = 8; // Vai para o ciclo de WRITEBACK
                    break;
                case 2: // SUB
                    printf("Instrução SUB\n");
                    registradores_estado->registradorSaidaALU = ula(registradores_estado->registradorA, registradores_estado->registradorB, 1);
                    estado = 8; // Vai para o ciclo de WRITEBACK
                    break;
                case 4: // AND
                    printf("Instrução AND\n");
                    registradores_estado->registradorSaidaALU = ula(registradores_estado->registradorA, registradores_estado->registradorB, 2);
                    estado = 8; // Vai para o ciclo de WRITEBACK
                    break;
                case 5: // OR
                    printf("Instrução OR\n");
                    registradores_estado->registradorSaidaALU = ula(registradores_estado->registradorA, registradores_estado->registradorB, 3);
                    estado = 8; // Vai para o ciclo de WRITEBACK
                    break;
                default:
                    printf("Função desconhecida\n");
                    estado = 0;
                    break;
            }
            break;

        case 8: // WRITEBACK (tipo R)
            printf("--------Executando ciclo WRITEBACK (TIPO R)-----------\n");
            // Escrever resultados de volta aos registradores (tipo R)
            banco_registradores->registradores[instrucao.rd] = registradores_estado->registradorSaidaALU;
            estado = 0;
            break;

        case 9: // Condição BEQ
            printf("--------Executando condição BEQ-----------\n");
            // Verificação de condição BEQ
            estado = 0;
            break;

        case 10: // Estado do tipo J
            printf("--------Executando ciclo EXECUTE (TIPO J)-----------\n");
            pc->endereco_atual = instrucao.addr;
            pc->endereco_proximo = pc->endereco_atual + 1;
            printf("Jump executado com sucesso\n");
            estado = 0;
            break;

        default:
            printf("Estado desconhecido\n");
            estado = 0;
            break;
    }
}


// FUNÇÕES DE INICIAÇÃO E ENTRADA
void inicializarBancoRegistradores(BancoRegistradores *banco_registradores) {
    for(int i = 0; i < TAM_REGISTRADORES; i++) {
        banco_registradores->registradores[i] = 0;
    }
}

void inicializarPC(PC *pc) {
    pc->endereco_atual = 0;
    pc->endereco_proximo = 1;
}

void inicializarMemoriaDados() {
    // Inicializa a memória de dados preenchendo ela com zeros
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        memoria_dados[i] = 0;
    }
}
// FUNÇÕES DE CARREGAR MEMORIA DE INSTRUÇÕES E DADOS
void carregarMemoriaUnica() {
    char nome_arquivo[50];
    int opcao;
    printf("Escolha entre carregar a memoria de instrucoes (0) ou a memoria de dados (1): ");
    scanf("%d", &opcao);
    printf("Digite o nome do arquivo: ");
    scanf("%s", nome_arquivo);

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    if (opcao == 0) {
        // Carrega a memória de instruções
        int i = 0;
        char linha[TAM_INSTRUCAO + 1];
        while (fgets(linha, TAM_INSTRUCAO + 1, arquivo) && i < TAM_MEMORIA_INSTRUCAO) {
            linha[strcspn(linha, "\n")] = '\0'; // Remove o caractere de nova linha
            strncpy(memoria_instrucao[i], linha, TAM_INSTRUCAO);
            i++;
        }
        printf("Memoria de instrucoes carregada com sucesso!\n");
    } else if (opcao == 1) {
        // Carrega a memória de dados
        int endereco, valor;
        // Lê cada par de valores endereço e valor do arquivo e carrega na memória de dados
        for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
            fscanf(arquivo, "Endereço de memoria[%d]:%d\n", &endereco, &valor);
            memoria_dados[endereco] = valor;
        }

        fclose(arquivo);
        printf("Memória de dados carregada com sucesso\n");
    } else {
        printf("Opcao inválida\n");
    }

    fclose(arquivo);
}

// FUNÇÕES DE SALVAMENTO
void salvar_asm() {
    FILE *arquivo_asm = fopen("programa.asm", "w"); //w = write / ponteiro para escrever no arquivo
    if (arquivo_asm == NULL) {
        printf("Erro ao criar o arquivo\n");
        return;
    }
    // Iterar sobre cada instrução na memória e converter para assembly
    for (int i = 0; i < TAM_MEMORIA; i++) {
        converter_asm(memoria_instrucao[i], arquivo_asm, codificarInstrucao(memoria_instrucao[i]));
        fprintf(arquivo_asm, "\n");
    }
    fclose(arquivo_asm);
    printf("Arquivo .asm salvo com sucesso!\n");
}

// Salvar configuração de memória em .data

void salvar_data() {
    FILE *arquivo_memoria = fopen("programa.data", "w"); //w = write / ponteiro para escrever no arquivo
    if (arquivo_memoria == NULL) {
        printf("Erro ao criar o arquivo\n");
        return;
    }
    // Iterar sobre cada endereço na memória de dados e salvar o conteúdo
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        fprintf(arquivo_memoria, "Endereço de memoria[%d]:%d\n", i, memoria_dados[i]);
	/* Verificar para não sobrecarregar 
	if (pc.endereco_atual >= TAM_MEMORIA_DADOS) {
            printf("Erro: endereco de memoria excedeu o limite\n");
            return;
        }*/
    }
    fclose(arquivo_memoria);
    printf("Arquivo .mem salvo com sucesso!\n");
}



//FUNÇÕES DE IMPRESSÃO
void imprimirInstrucao(Instrucao inst) {
    switch (inst.tipo) {
        case R_TYPE:
            printf("Tipo: [TIPO R]\n");
            printf("Opcode: [%d]\n", inst.opcode);
            printf("Rs: [%d]\n", inst.rs);
            printf("Rt: [%d]\n", inst.rt);
            printf("Rd: [%d]\n", inst.rd);
            printf("Funct: [%d]\n", inst.funct);
            printf("--------------------\n");
            break;
        case I_TYPE:
            printf("Tipo: [TIPO I]\n");
            printf("Opcode: [%d]\n", inst.opcode);
            printf("Rs: [%d]\n", inst.rs);
            printf("Rt: [%d]\n", inst.rt);
            printf("Imediato: [%d]\n", inst.imm);
            printf("--------------------\n");
            break;
        case J_TYPE:
            printf("Tipo: [TIPO J]\n");
            printf("Opcode: [%d]\nt", inst.opcode);
            printf("Addr: [%d]\n", inst.addr);
            printf("--------------------\n");
            break;
    }
}

void imprimirMemoriaUnica() {
    printf("Memoria de Instrucoes:\n");
    for (int i = 0; i < TAM_MEMORIA_INSTRUCAO; i++) {
        printf("Instrucao[%d]: %s\n", i, memoria_instrucao[i]);
    }

    printf("\nMemoria de Dados:\n");
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        printf("Dados[%d]: %d\n", i, memoria_dados[i]);
    }
}

void imprimirRegistradores(BancoRegistradores *banco_registradores) {
    printf("Conteudo do banco de registradores:\n");
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, banco_registradores->registradores[i]);
    }
}



//BACK
void inicializePilha(struct descritor* desc){
    desc->topo = NULL;
}

void save_backup(PC* pc, struct descritor *topo, BancoRegistradores *banco_registradores) {
    struct nodo *novoNodo = (struct nodo*)malloc(sizeof(struct nodo));
    if (novoNodo == NULL) {
        printf("Erro ao alocar memória para o backup.\n");
        return;
    }
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        novoNodo->banco_undo.registradores[i] = banco_registradores->registradores[i];
    }
    for (int i = 0; i < TAM_MEMORIA_INSTRUCAO; i++) {
        strcpy(novoNodo->mem_undo.memoria.instrucoes[i], memoria_instrucao[i]);
    }
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        novoNodo->mem_undo.memoria.dados[i] = memoria_dados[i];
    }
    novoNodo->pc_undo.endereco_atual = pc->endereco_atual;
    novoNodo->pc_undo.endereco_proximo = pc->endereco_proximo;
    novoNodo->prox = topo->topo;
    topo->topo = novoNodo;
}

void undo(PC *pc, struct descritor *topo, BancoRegistradores *banco_registradores) {
    if (topo->topo == NULL) {
        printf("Erro: Nenhum backup disponível.\n");
        return;
    }
    struct nodo *nodoRemovido = topo->topo;
    topo->topo = topo->topo->prox;

    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        banco_registradores->registradores[i] = nodoRemovido->banco_undo.registradores[i];
    }
    for (int i = 0; i < TAM_MEMORIA_INSTRUCAO; i++) {
        strcpy(memoria_instrucao[i], nodoRemovido->mem_undo.memoria.instrucoes[i]);
    }
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        memoria_dados[i] = nodoRemovido->mem_undo.memoria.dados[i];
    }

    pc->endereco_atual = nodoRemovido->pc_undo.endereco_atual;
    pc->endereco_proximo = nodoRemovido->pc_undo.endereco_proximo;

    free(nodoRemovido);
    printf("Estado restaurado para o backup.\n");
}


