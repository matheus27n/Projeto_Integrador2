#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "minimips.h"

// FUNÇÕES GERAIS

char memoria_instrucao[TAM_MEMORIA][TAM_INSTRUCAO]; // Definição das variáveis
int memoria_dados[TAM_MEMORIA_DADOS]; // Definição das variáveis

int menu(){
    int m;
    printf("\n================================\n");
    printf("\t MINI-MIPS 8 BITS - UNIPAMPA\n"); 
    printf("\n================================\n\n");
    printf("1. Carregar memoria\n"); 
    printf("2. Carregar memoria de dados\n");
    printf("3. Imprimir memoria \n");
    printf("4. Imprimir memoria de dados\n");
    printf("5. Imprimir registradores \n"); 
    printf("6. Imprimir todo o simulador \n"); 
    printf("7. Salvar .asm \n"); 
    printf("8. Salvar .data \n"); 
    printf("9. Executa Programa (run)\n"); 
    printf("10. Executa uma instrucao (Step)\n"); 
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
void executarInstrucao(Instrucao inst, BancoRegistradores *banco_registradores, PC *pc) {
    int referencia = pc->endereco_atual; // Salva uma referencia caso algum procedimento não ocorra corretamente
    int endereco;
    int rd_teste;
    int rs_teste;
    int rt_teste;
    int resultado;

    switch(inst.tipo) {
        case R_TYPE:
            // Execução das instruções do tipo R
            switch (inst.funct) {
                case 0: // add
                    rd_teste = banco_registradores->registradores[inst.rd];
                    rs_teste = banco_registradores->registradores[inst.rs];
                    rt_teste = banco_registradores->registradores[inst.rt];
                    
                    resultado = rs_teste + rt_teste;
                    
                    if (rs_teste & 0x80) { // Verifica se o bit mais significativo é 1
                        rs_teste = rs_teste | 0xFFFFFF00; // Extende o sinal para 32 bits
                    }
                    if (rt_teste & 0x80) { // Verifica se o bit mais significativo é 1
                        rt_teste = rt_teste | 0xFFFFFF00; // Extende o sinal para 32 bits
                    }
                    
                    resultado = rs_teste + rt_teste;
                    
                    if ((resultado > 127) || (resultado < -128)) {
                        printf("Overflow ocorreu durante a operacao!\n");
                    } else {
                        banco_registradores->registradores[inst.rd] = resultado;
                    }
                    break;

                case 2: // sub
                    rd_teste = banco_registradores->registradores[inst.rd];
                    rs_teste = banco_registradores->registradores[inst.rs];
                    rt_teste = banco_registradores->registradores[inst.rt];
                    
                    resultado = rs_teste - rt_teste;

                    if (rt_teste & 0x80) { // Verifica se o bit mais significativo é 1
                        rt_teste = ~rt_teste + 1; // Calcula o complemento de B
                    }
                    
                    resultado = rs_teste - rt_teste;
                    
                    if ((resultado > 127) || (resultado < -128)) {
                        printf("Overflow ocorreu durante a operacao!\n");
                    } else {
                        banco_registradores->registradores[inst.rd] = resultado;
                    }
                    break;

                case 4: // and
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] & banco_registradores->registradores[inst.rt];
                    break;

                case 5: // or
                    banco_registradores->registradores[inst.rd] = banco_registradores->registradores[inst.rs] | banco_registradores->registradores[inst.rt];
                    break;
                    
                default:
                    printf("Funcao R nao reconhecida: %d", inst.funct);
                    break;
            }
            break;

        case I_TYPE:
            // Execução das instruções do tipo I
            switch (inst.opcode) {
                case 4: // addi
                    rs_teste = banco_registradores->registradores[inst.rs];
                    rd_teste = rs_teste + inst.imm;
                    
                    // Verificação de overflow pode ser opcional dependendo da especificação da arquitetura
                    if ((rd_teste > 127) || (rd_teste < -128)) { // Supondo uma arquitetura de 8 bits, ajuste conforme necessário
                        printf("Overflow ocorreu durante a operacao!\n");
                    } else {
                        banco_registradores->registradores[inst.rt] = rd_teste;
                    }
                break;

                 case 11: // Load word
                    endereco = banco_registradores->registradores[inst.rs] + inst.imm; // Endereço é a soma do registrador rs com o imediato
                    banco_registradores->registradores[inst.rt] = memoria_dados[endereco]; // Carrega o dado para o registrador rt
                    break;

                case 15: // Store word
                    endereco = banco_registradores->registradores[inst.rs] + inst.imm;
                    
                    if (endereco >= 0 && endereco < TAM_MEMORIA_DADOS) {
                        memoria_dados[endereco] = banco_registradores->registradores[inst.rt];
                    } else {
                        printf("Erro: Endereço de memória inválido");
                    }
                    break;

                case 8: // Branch on equal
                  // Primeiro, verifica se os valores dos registradores rs e rt são iguais
                    if (banco_registradores->registradores[inst.rs] == banco_registradores->registradores[inst.rt]) {
                        // Se forem iguais, calcula o novo endereço
                        endereco = pc->endereco_atual + (inst.imm * 4); // Multiplica offset por 4 para obter o deslocamento em bytes

                        // Verifica se o novo endereço está dentro dos limites da memória
                        if(endereco >= 0 && endereco < TAM_INSTRUCAO){
                            pc->endereco_atual = endereco;
                        } else {
                            printf("Erro: Endereço de memória inválido");
                        }
                    }else{
                        pc->endereco_atual = referencia;
                    }
            }
            break;

        case J_TYPE:
            pc->endereco_atual = inst.addr; // Atualiza o PC para o endereço especificado na instrução
            
            if (pc->endereco_atual >= 0 && pc->endereco_atual < TAM_MEMORIA) { // Verifica se o endereço é válido
                printf("Desvio para o endereço: %d", pc->endereco_atual);
                pc->endereco_proximo = pc->endereco_atual; // Avança para a próxima instrução na sequência
            } else {
                pc->endereco_atual = referencia; // Define o próximo endereço como inválido e retorna
            }
            break;
    }

    pc->endereco_atual = pc->endereco_proximo; // Atualizar o endereço atual
    pc->endereco_proximo++; // Avançar para a próxima instrução na sequência
    
    printf("\nPC atual: %d", pc->endereco_atual);
    printf("PC próximo: %d", pc->endereco_proximo);
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


