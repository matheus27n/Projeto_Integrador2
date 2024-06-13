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
    printf("5. Decodificação de Instruções \n");
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
    imprimirInstrucao(inst); // Chama a função para imprimir os campos da instrução
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
void executarCicloInstrucao(PC *pc, BancoRegistradores *banco_registradores, Controle *unidadeControle, Instrucao inst, EstadoCiclo estado_ciclo) {
    int referencia = pc->endereco_atual; // Salva o endereço atual para referência
    int endereco;
    int rd_teste;
    int rs_teste;
    int rt_teste;
    int resultado;

    switch (estado_ciclo) {
        case FETCH:
            // Busca a instrução na memória de instruções
            strcpy(memoria_instrucao[pc->endereco_atual], memoria_instrucao[pc->endereco_atual]);
            pc->endereco_atual = pc->endereco_proximo;
            pc->endereco_proximo++;
            printf("A instrucao %s foi buscada na memoria de instrucoes\n", memoria_instrucao[referencia]);
            printf("fetch\n");
            break;
        case DECODE:
            // Decodifica a instrução
            inst = codificarInstrucao(memoria_instrucao[referencia]);
            printf("decode\n");
            break;
        case EXECUTE:
            // Executa a instrução
            switch (inst.tipo) {
                case R_TYPE:
                    // Verifica se os registradores existem
                    rd_teste = banco_registradores->registradores[inst.rd];
                    rs_teste = banco_registradores->registradores[inst.rs];
                    rt_teste = banco_registradores->registradores[inst.rt];
                    if (rd_teste == 0 && rs_teste == 0 && rt_teste == 0) {
                        break;
                    }
                    // Executa a instrução
                    resultado = ula(banco_registradores->registradores[inst.rs], banco_registradores->registradores[inst.rt], inst.funct);
                    // Verifica se houve overflow
                    if (check_overflow(resultado)) {
                        printf("Erro: Overflow detectado\n");
                        break;
                    }
                    // Atualiza o registrador de destino
                    banco_registradores->registradores[inst.rd] = resultado;
                    break;
                case I_TYPE:
                    switch (inst.opcode) {
                        case 4: // addi
                            // Verifica se os registradores existem
                            rd_teste = banco_registradores->registradores[inst.rt];
                            rs_teste = banco_registradores->registradores[inst.rs];
                            if (rd_teste == 0 && rs_teste == 0) {
                                break;
                            }
                            // Executa a instrução
                            resultado = ula(banco_registradores->registradores[inst.rs], inst.imm, 0);
                            // Verifica se houve overflow
                            if (check_overflow(resultado)) {
                                printf("Erro: Overflow detectado\n");
                                break;
                            }
                            // Atualiza o registrador de destino
                            banco_registradores->registradores[inst.rt] = resultado;
       
}
        }
    }
}

void setControleSignal(EstadoCiclo *estadoCiclo, Instrucao *inst, Controle *unidadeControle){
    switch(*estadoCiclo){ // Usando *estadoCiclo para acessar o valor apontado pelo ponteiro
        case FETCH:
            unidadeControle->LeMem = 0;
            unidadeControle->IouD = 0;
            unidadeControle->EscreveIR = 1;
            unidadeControle->OrigB = 1;
            unidadeControle->OrigA = 0;
            unidadeControle->EscrevePc = 1;
            unidadeControle->OrigPC = 0;
            break;

        case DECODE:
            unidadeControle->OrigA = 0;
            unidadeControle->OrigB = 3;
            unidadeControle->OpAlu = 0;
            break;

        case EXECUTE:
            switch(inst->tipo){
                case R_TYPE:
                    unidadeControle->OrigA = 1;
                    unidadeControle->OrigB = 2;
                    unidadeControle->OpAlu = inst->funct;
                    unidadeControle->RegDst = 1;
                    break;
                case I_TYPE:
                    switch (inst->opcode){
                        case 4: //addi
                            unidadeControle->OrigA = 1; //rs
                            unidadeControle->OrigB = 0; //imm
                            unidadeControle->OpAlu = 0; //add
                            unidadeControle->RegDst = 0; //rt
                            unidadeControle->EscreveMem = 1; //write
                            unidadeControle->EscreveReg = 1; //write
                            unidadeControle->RegDst = 0; //rt
                            unidadeControle->MemParaReg = 0; //mem para reg
                            break;
                        
                        case 11: //lw
                            unidadeControle->OrigA = 1;
                            unidadeControle->OrigB = 0;
                            unidadeControle->OpAlu = 0;
                            unidadeControle->RegDst = 0;
                            unidadeControle->EscreveMem = 0;
                            unidadeControle->IouD = 1;
                            break;
                        
                        case 15: //sw
                            unidadeControle->OrigA = 1;
                            unidadeControle->OrigB = 0;
                            unidadeControle->OpAlu = 0;
                            unidadeControle->RegDst = 0;
                            break;

                        case 8: //beq
                            unidadeControle->OrigA = 1;
                            unidadeControle->OrigB = 0;
                            unidadeControle->OpAlu = 1;
                            unidadeControle->RegDst = 0;
                            break;
                    }
            }
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
void carregarMemoriaUnica(Controle *unidadeControle) {
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
struct nodo* save_backup(PC* pc, int memoria_dados[], BancoRegistradores *banco_registradores) {
    struct nodo *novoNodo = (struct nodo*)malloc(sizeof(struct nodo));
    if (novoNodo == NULL) {
        printf("Erro ao alocar memória para o backup");
        return NULL;
    }

    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        novoNodo->banco_undo.registradores[i] = banco_registradores->registradores[i];
    }

    novoNodo->pc_undo.endereco_atual = pc->endereco_atual;
    novoNodo->pc_undo.endereco_proximo = pc->endereco_proximo;

    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        novoNodo->mem_dados_undo[i] = memoria_dados[i];
    }

    return novoNodo;
}

void undo(struct nodo *backup, PC *pc, int *memoria_dados, BancoRegistradores *banco_registradores) {
    if (backup == NULL) {
        printf("Erro: Backup inválido");
        return;
    }

    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        banco_registradores->registradores[i] = backup->banco_undo.registradores[i];
    }

    pc->endereco_atual = backup->pc_undo.endereco_atual;
    pc->endereco_proximo = backup->pc_undo.endereco_proximo;

    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        memoria_dados[i] = backup->mem_dados_undo[i];
    }

    printf("Estado restaurado para o backup.\n");
    //mostrar instrução que voltou 
    codificarInstrucao(memoria_instrucao[pc->endereco_atual]);
    free(backup); // Libere a memória alocada para o backup
}


