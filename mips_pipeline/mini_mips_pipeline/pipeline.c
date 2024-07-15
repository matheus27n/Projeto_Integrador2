#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "pipeline.h"

// FUNÇÕES GERAIS

char memoria_instrucao[TAM_MEMORIA][TAM_INSTRUCAO]; // Definição das variáveis
int memoria_dados[TAM_MEMORIA_DADOS]; // Definição das variáveis

int menu(){
    int m;
    printf("\n================================\n");
    printf("\t MINI-MIPS PIPELINE- UNIPAMPA\n"); 
    printf("\n================================\n\n");
    printf("1. Carregar memoria\n"); 
    printf("2. Carregar memoria de dados\n");
    printf("3. Imprimir memoria de instruções \n");
    printf("4. Imprimir memoria de dados\n");
    printf("5. Imprimir registradores \n"); 
    printf("6. Imprimir todo o simulador \n"); 
    printf("7. Salvar .asm \n"); 
    printf("8. Salvar .data \n"); 
    printf("9. Executa Programa (run)\n"); 
    printf("10. Executa uma instrucao (Step pipeline)\n"); 
    printf("11. Volta uma instrucao (Back)\n"); 
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
void carregarMemoriaDados() {
    char nome_arquivo[50];
    printf("Digite o nome do arquivo .data que deseja abrir: ");
    scanf("%s", nome_arquivo);

    FILE *arquivo_memoria = fopen(nome_arquivo, "r"); // Abre o arquivo especificado pelo usuário
    if (arquivo_memoria == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    int endereco, valor;
    // Lê cada par de valores endereço e valor do arquivo e carrega na memória de dados
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        fscanf(arquivo_memoria, "Endereço de memoria[%d]:%d\n", &endereco, &valor);
        memoria_dados[endereco] = valor;
    }

    fclose(arquivo_memoria);
    printf("Memória de dados carregada com sucesso\n");
}

void carregarMemoria() {
    char nome_arquivo[50];
    printf("Digite o nome do arquivo .mem que deseja abrir: ");
    scanf("%s", nome_arquivo);

    FILE *arquivo_memoria = fopen(nome_arquivo, "r"); // Abre o arquivo especificado pelo usuário
    if (arquivo_memoria == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return;
    }
    /* Verificar para não sobrecarregar o PC
	 for (int i = 0; i < TAM_MEMORIA; i++) {
        ...
        if (pc.endereco_atual >= TAM_MEMORIA) {
            printf("Erro: endereco de memoria excedeu o limite\n");
            return;
        }
    */
    int i = 0; // Começa a partir do endereço 0
    char linha[TAM_INSTRUCAO + 1]; // +1 para o caractere nulo
    while (fgets(linha, TAM_INSTRUCAO + 1, arquivo_memoria)) { 
        char *pos; // Ponteiro para a posição do caractere de nova linha
        if ((pos = strchr(linha, '\n')) != NULL) { // Busca o caractere nulo na string linha
            *pos = '\0'; // Substitui o caractere de nova linha por nulo
        }
        strncpy(memoria_instrucao[i], linha, TAM_INSTRUCAO); // Copia a linha para a memória de instrução
        i++;
    }
    fclose(arquivo_memoria);
    printf("Memoria carregada com sucesso\n");
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

void imprimirMemoria(char memoria_instrucao[][TAM_INSTRUCAO]) {
    printf("Conteudo da memoria:\n");
    for (int i = 0; i < 11; i++) {
        printf("Endereco %d: %s\n", i, memoria_instrucao[i]);
        // Decodifica e imprime a instrução
        codificarInstrucao(memoria_instrucao[i]);
        printf("\n\n");
    }
}

void imprimirMemoriaDados() {
    printf("Conteudo da memoria de dados:\n");
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        printf("Endereco %d: %d\n", i, memoria_dados[i]);
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





//NOVAS FUNCIONALIDADES PIPELINE
// Implemente as funções para executar o programa e as instruções
void executarPipeline(PC *pc, BancoRegistradores *bancoRegistrador, Registrador_IF_ID *if_id, Instrucao *inst, Registrador_ID_EX *id_ex, Registrador_EX_MEM *ex_mem, Registrador_MEM_WB *mem_wb) {
    static int estados = 0; // Mantém o estado do ciclo entre as chamadas da função

    switch (estados) {
        case 0:
            printf("[ESTADO 1]\n");
            estagio_IF(pc, if_id, inst);
            estados = 1;
            break;
        case 1:
            printf("[ESTADO 2]\n");
            estagio_ID(if_id, id_ex, bancoRegistrador, inst);
            estagio_IF(pc, if_id, inst);
            estados = 2;
            break;
        case 2:
            printf("[ESTADO 3]\n");
            estagio_EX(id_ex, ex_mem,if_id, bancoRegistrador, inst);
            estagio_ID(if_id, id_ex, bancoRegistrador, inst);
            estagio_IF(pc, if_id, inst);
            estados = 3;
            break;
        case 3:
            printf("[ESTADO 4]\n");
            estagio_MEM(ex_mem, mem_wb, if_id, bancoRegistrador);
            estagio_EX(id_ex, ex_mem,if_id, bancoRegistrador, inst);
            estagio_ID(if_id, id_ex, bancoRegistrador, inst);
            estagio_IF(pc, if_id, inst);
            estados = 4;
            break;
        case 4:
            printf("[ESTADO 5]\n");
            estagio_WB(mem_wb, if_id, bancoRegistrador);
            estagio_MEM(ex_mem, mem_wb, if_id, bancoRegistrador);
            estagio_EX(id_ex, ex_mem,if_id, bancoRegistrador, inst);
            estagio_ID(if_id, id_ex, bancoRegistrador, inst);
            estagio_IF(pc, if_id, inst);
            estados = 0; // Reseta para o início ou ajusta conforme necessário
            break;
        case 5:
            printf("[ESTADO 6]\n");
            estagio_WB(mem_wb, if_id, bancoRegistrador);
            estagio_MEM(ex_mem, mem_wb, if_id, bancoRegistrador);
            estagio_EX(id_ex, ex_mem,if_id, bancoRegistrador, inst);
            estagio_ID(if_id, id_ex, bancoRegistrador, inst);
            estagio_IF(pc, if_id, inst);
            estados = 0; // Reseta para o início ou ajusta conforme necessário
            break;
    }
}



void executarTodoPipeline(){
    
}

void estagio_IF(PC *pc, Registrador_IF_ID *if_id, Instrucao *instrucao) {
    printf("-----------------------------\n");
    printf("Executando estagio IF...\n");
    // Busca a instrução na memória de instruções
    char *instrucao_binaria = memoria_instrucao[pc->endereco_atual];
    // Converte a instrução binária para a estrutura Instrucao
    *instrucao = codificarInstrucao(instrucao_binaria);
    printf("Instrucao buscada e: %s\n", instrucao_binaria);
    printf("\n");
    // Salva a instrução no registrador IF/ID
    //variavel auxiliar para armazenar a instrução que está no registrador IF_ID anterior
    if_id->instrucao = *instrucao;
    if_id->pc = pc->endereco_atual;
    // Atualiza o PC
    pc->endereco_atual = pc->endereco_proximo;
    pc->endereco_proximo++;

    //quero imprimir oque tem no registrador IF_ID
    printf("[Conteudo do registrador IF_ID]\n");
    printf("Instrucao:");
    //instrução que contem em assembly
    converter_asm(memoria_instrucao[if_id->pc], stdout, *instrucao);
    printf("\n");

    //printf("PC: %d\n", if_id->pc);
    printf("\n");
}


void estagio_ID(Registrador_IF_ID *if_id, Registrador_ID_EX *id_ex, BancoRegistradores *banco_registradores, Instrucao *instrucao) {
    printf("-----------------------------\n");
    printf("Executando estagio ID...\n");

    // Busca a instrução no registrador IF/ID
    *instrucao = if_id->instrucao;
    
    // Ajuste na linha abaixo: corrigindo a busca na memória de instruções
    Instrucao instrucao_atual = codificarInstrucao(memoria_instrucao[if_id->pc-1]);

    // Decodifica a instrução buscada.
    printf("Identificando instrucao: \n");
    imprimirInstrucao(*instrucao);

    // Lê os registradores fonte do banco de registradores.
    int reg_rs = banco_registradores->registradores[instrucao->rs];
    int reg_rt = banco_registradores->registradores[instrucao->rt];

    // Calcula o endereço do próximo PC para instruções de desvio condicional.
    int pc_proximo = if_id->pc + 1;
    if (instrucao->opcode == 8) {
        pc_proximo = if_id->pc + 1 + instrucao->imm;
    }

    printf("\n");
}

void estagio_EX(Registrador_ID_EX *id_ex, Registrador_EX_MEM *ex_mem, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores, Instrucao *instrucao) {
    printf("-----------------------------\n");
    printf("Executando estagio EX...\n");
    // Busca a instrução no registrador ID/EX
    //*instrucao = id_ex->instrucao;
    printf("Executando a intrucao:");
    Instrucao instrucao_atual = codificarInstrucao(memoria_instrucao[if_id->pc - 1]);
    converter_asm(memoria_instrucao[if_id->pc], stdout, instrucao_atual);
    printf("\n");

    // Executa a operação aritmética ou lógica indicada pela instrução.
    int resultado = 0;
    switch (instrucao_atual.tipo) {
        case R_TYPE:
            switch (instrucao_atual.funct) {
                case 0: // ADD
                    resultado = ula(banco_registradores->registradores[instrucao_atual.rs], banco_registradores->registradores[instrucao_atual.rt], 0);
                    break;
                case 2: // SUB
                    resultado = ula(banco_registradores->registradores[instrucao_atual.rs], banco_registradores->registradores[instrucao_atual.rt], 1);
                    break;
                case 4: // AND
                    resultado = ula(banco_registradores->registradores[instrucao_atual.rs], banco_registradores->registradores[instrucao_atual.rt], 2);
                    break;
                case 5: // OR
                    resultado = ula(banco_registradores->registradores[instrucao_atual.rs], banco_registradores->registradores[instrucao_atual.rt], 3);
                    break;
                default:
                    printf("Funcao R nao reconhecida: %d\n", instrucao_atual.funct);
                    break;
            }
            break;
        case I_TYPE:
            switch (instrucao_atual.opcode) {
                case 4: // ADDI
                    resultado = ula(banco_registradores->registradores[instrucao_atual.rs], instrucao_atual.imm, 0);
                    break;
                case 11: // LW
                    resultado = banco_registradores->registradores[instrucao_atual.rs] + instrucao_atual.imm;
                    break;
                case 15: // SW
                    resultado = banco_registradores->registradores[instrucao_atual.rs] + instrucao_atual.imm;
                    break;
                case 8: // BEQ
                    resultado = banco_registradores->registradores[instrucao_atual.rs] - banco_registradores->registradores[instrucao_atual.rt];
                    break;
                default:
                    printf("Opcode I nao reconhecido: %d\n", instrucao_atual.opcode);
                    break;
            }
            break;
        case J_TYPE:
            // Adicione o código aqui para o tipo J, se necessário
            break;
        default:
            printf("Tipo de instrucao nao reconhecido: %d\n", instrucao_atual.tipo);
            break;
    }

}


//O estágio MEM é responsável por acessar a memória de dados. Dependendo do tipo de instrução, ele pode ler ou escrever dados na memória.
void estagio_MEM(Registrador_EX_MEM *ex_mem, Registrador_MEM_WB *mem_w, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores) {
    printf("-----------------------------\n");
    printf("Executando estagio MEM...\n");
    Instrucao instrucao_atual = codificarInstrucao(memoria_instrucao[if_id->pc - 2]);
    converter_asm(memoria_instrucao[if_id->pc], stdout, instrucao_atual);
    printf("\n");

    if(instrucao_atual.tipo == I_TYPE){
        switch (instrucao_atual.opcode) {
            case 11: // LW
                banco_registradores->registradores[instrucao_atual.rt] = banco_registradores->registradores[instrucao_atual.rs] + instrucao_atual.imm;
                printf("O endereco da memoria lido pelo LW e: [%d]\n",instrucao_atual.rt );
                break;
            case 15: // SW
                banco_registradores->registradores[instrucao_atual.rt] = banco_registradores->registradores[instrucao_atual.rs] + instrucao_atual.imm;
                printf("O valor escrito na memoria de dados é: [%d]\n",instrucao_atual.rt );
                break;
            default:
                printf("Opcode I nao reconhecido: %d\n", instrucao_atual.opcode);
                break;
        }
    }


}


//O estágio WB é responsável por escrever o resultado de volta no banco de registradores.
void estagio_WB(Registrador_MEM_WB *mem_wb, Registrador_IF_ID *if_id, BancoRegistradores *banco_registradores) {
    printf("-----------------------------\n");
    printf("Executando estagio WB...\n");

    Instrucao instrucao_atual = codificarInstrucao(memoria_instrucao[if_id->pc - 2]);
    converter_asm(memoria_instrucao[if_id->pc], stdout, instrucao_atual);
    printf("\n");

        if(instrucao_atual.tipo == I_TYPE){
        switch (instrucao_atual.opcode) {
            case 11: // LW
                printf("LW chegou na parte de write back\n");
                break;
            case 15: // SW
                banco_registradores->registradores[instrucao_atual.rt] = banco_registradores->registradores[instrucao_atual.rs] + instrucao_atual.imm;
                printf("O valor escrito na memoria de dados é: [%d]\n",instrucao_atual.rt );
                break;
            default:
                printf("Opcode I nao reconhecido: %d\n", instrucao_atual.opcode);
                break;
        }
    }
}
