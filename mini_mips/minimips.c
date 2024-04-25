#include <string.h>
#include <stdlib.h>
#include "minimips.h"

// FUNÇÕES GERAIS

char memoria_instrucao[TAM_MEMORIA][TAM_INSTRUCAO]; // Definição das variáveis
int memoria_dados[TAM_MEMORIA_DADOS]; // Definição das variáveis

int menu(){
    int m;
    printf("\n================================\n");
    printf("\tMENU PRINCIPAL"); 
    printf("\n================================\n\n");
    printf("1. Carregar memoria\n"); 
    printf("2. Imprimir memoria \n"); 
    printf("3. Imprimir registradores \n"); 
    printf("4. Imprimir todo o simulador \n"); 
    printf("5. Salvar .asm \n"); 
    printf("6. Salvar .data \n"); 
    printf("7. Executa Programa (run)\n"); 
    printf("8. Executa uma instrucao (Step)\n"); 
    printf("9. Volta uma instrucao (Back)\n"); 
    printf("10.Carregar memoria de dados\n");
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
    switch (inst.tipo) {
        case R_TYPE:
            switch (inst.funct) {
                case 0:
                    fprintf(arquivo_asm, "add R%d, R%d, R%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 2:
                    fprintf(arquivo_asm, "sub R%d, R%d, R%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 4:
                    fprintf(arquivo_asm, "and R%d, R%d, R%d", inst.rd, inst.rs, inst.rt);
                    break;
                case 5:
                    fprintf(arquivo_asm, "or R%d, R%d, R%d", inst.rd, inst.rs, inst.rt);
                    break;
                default:
                    fprintf(arquivo_asm, "Funcao R nao reconhecida: %d", inst.funct);
                    break;
            }
            break;
        case I_TYPE:
            switch (inst.opcode) {
                case 4:
                    fprintf(arquivo_asm, "addi R%d, R%d, %d", inst.rt, inst.rs, inst.imm);
                    break;
                case 11:
                    fprintf(arquivo_asm, "lw R%d, %d(R%d)", inst.rt, inst.imm, inst.rs);
                    break;
                case 15:
                    fprintf(arquivo_asm, "sw R%d, %d(R%d)", inst.rt, inst.imm, inst.rs);
                    break;
                case 8:
                    fprintf(arquivo_asm, "beq R%d, R%d, %d", inst.rt, inst.rs, inst.imm);
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

// Implemente as funções para executar o programa e as instruções
void executarInstrucao(Instrucao inst, BancoRegistradores *banco_registradores, PC *pc) {
    int referencia = pc->endereco_atual; // Salva uma referencia caso algum procedimento não ocorra corretamente
    int endereco;
    switch(inst.tipo) {
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
                default:
                    printf("Funcao R nao reconhecida: %d\n", inst.funct);
                    break;
            }
            break;
        case I_TYPE:
            // Execução das instruções do tipo I
            switch (inst.opcode) {
                case 4: // addi
                    banco_registradores->registradores[inst.rt] = banco_registradores->registradores[inst.rs] + inst.imm;
                    memoria_dados[inst.rt] = banco_registradores->registradores[inst.rt]; // Atualizar a memória de dados
                    break;
                case 11: //Load word
                    endereco = banco_registradores->registradores[inst.rs] + inst.imm; // Endereço é a soma do registrador rs com o imediato
                    banco_registradores->registradores[inst.rt] = memoria_dados[endereco]; // Carrega o dado para o registrador rt
                    break;
                case 15: //Store word
                    endereco = banco_registradores->registradores[inst.rs] + inst.imm; // Endereço é a soma do registrador rs com o imediato
                    memoria_dados[endereco] = banco_registradores->registradores[inst.rt]; // Armazena o dado do registrador rt na memória de dados
                    //zerar o registrador após transferir os dados para a memoria de dados
                    banco_registradores->registradores[inst.rt] = 0;
                    break;
                case 8: // Brench on equal
                    if (banco_registradores->registradores[inst.rt] == banco_registradores->registradores[inst.rs]) {
                        pc->endereco_atual = inst.imm; // Atualiza o PC para o endereço especificado pelo imediato
                    } else {
                        pc->endereco_atual = referencia;
                    }
                    break;
            }
            break;
        case J_TYPE:
            pc->endereco_atual = inst.addr; // Atualiza o PC para o endereço especificado na instrução
            if (pc->endereco_atual >= 0 && pc->endereco_atual < TAM_MEMORIA) { // Verifica se o endereço é válido
                printf("Desvio para o endereco: %d\n", pc->endereco_atual);
                pc->endereco_proximo = pc->endereco_atual; // Avança para a próxima instrução na sequência
            } else {
                pc->endereco_atual = referencia; // Define o próximo endereço como inválido e retorna
            }
            break;
    }
    pc->endereco_atual = pc->endereco_proximo; // Atualizar o endereço atual
    pc->endereco_proximo++; // Avançar para a próxima instrução na sequência
    printf("\nPC atual: %d\n", pc->endereco_atual);
    printf("PC proximo: %d\n", pc->endereco_proximo);

    // Criar o backup após a execução da instrução e armazenar o ponteiro para ele
    Backup backup = back(banco_registradores, pc, &inst);

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
        //void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm, Instrucao inst)
        converter_asm(memoria_instrucao[i], arquivo_asm, codificarInstrucao(memoria_instrucao[i]));
        fprintf(arquivo_asm, "\n");
    }

    fclose(arquivo_asm);
    printf("Arquivo .asm salvo com sucesso!\n");
}

// Salvar configuração de memória em .mem 
void salvar_data() {
    FILE *arquivo_memoria = fopen("programa.data", "w"); //w = write / ponteiro para escrever no arquivo
    if (arquivo_memoria == NULL) {
        printf("Erro ao criar o arquivo\n");
        return;
    }
    // Iterar sobre cada endereço na memória de dados e salvar o conteúdo
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        fprintf(arquivo_memoria, "Endereço de memoria[%d]:%d\n", i, memoria_dados[i]);
    }
    fclose(arquivo_memoria);
    printf("Arquivo .mem salvo com sucesso!\n");
}

// FUNÇÃO DE BACK (VOLTAR 1 INSTRUÇÃO)


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
    for (int i = 0; i < /*TAM_MEMORIA_DADOS*/ 16; i++) {
        printf("Endereco %d: %s\n", i, memoria_instrucao[i]);
        // Decodifica e imprime a instrução
        codificarInstrucao(memoria_instrucao[i]);
        printf("\n\n");
    }
}

void imprimirMemoriaDados() {
    printf("Conteudo da memoria de dados:\n");
    for (int i = 0; i < /*TAM_MEMORIA_DADOS*/ 16; i++) {
        printf("Endereco %d: [%d]\n", i, memoria_dados[i]);
    }
}

void imprimirRegistradores(BancoRegistradores *banco_registradores) {
    printf("Conteudo do banco de registradores:\n");
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, banco_registradores->registradores[i]);
    }
}



//BACK
Backup back(BancoRegistradores *banco_registradores, PC *pc, Instrucao *inst){
    Backup backup;
    backup.banco_registradores_backup = *banco_registradores;
    backup.pc_backup = *pc;
    backup.inst_backup = *inst;
    printf("Backup realizado com sucesso\n");
    //printar o backup na tela
    printf("Banco de registradores: \n");
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, backup.banco_registradores_backup.registradores[i]);
    }
    printf("PC atual: %d\n", backup.pc_backup.endereco_atual);
    printf("PC proximo: %d\n", backup.pc_backup.endereco_proximo);
    imprimirInstrucao(backup.inst_backup);
    return backup;
}

//FUNÇÃO DE VOLTA UMA INSTRUÇÃO
void voltarSimulador(BancoRegistradores *banco_registradores, PC *pc, Instrucao *inst, Backup *backup){
    *banco_registradores = backup->banco_registradores_backup;
    *pc = backup->pc_backup;
    *inst = backup->inst_backup;
    printf("Simulador voltado com sucesso\n");

    printf("Banco de registradores: \n");
    for (int i = 0; i < TAM_REGISTRADORES; i++) {
        printf("R%d: %d\n", i, banco_registradores->registradores[i]);
    }
    printf("PC atual: %d\n", pc->endereco_atual);
    printf("PC proximo: %d\n", pc->endereco_proximo);
    imprimirInstrucao(*inst);
    
}


