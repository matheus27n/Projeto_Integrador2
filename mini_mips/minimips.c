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
    printf("6. Salvar .mem \n"); 
    printf("7. Executa Programa (run)\n"); 
    printf("8. Executa uma instrucao (Step)\n"); 
    printf("9. Volta uma instrucao (Back)\n"); 
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
void converter_asm(char instrucao_binaria[TAM_INSTRUCAO], FILE *arquivo_asm) {
    // Extrai o opcode da instrução binária
    //pegar a instrução e converter para decimal apenas os 4 primeiros bits
    int opcode = (instrucao_binaria[0] - '0') * 8 + (instrucao_binaria[1] - '0') * 4 + (instrucao_binaria[2] - '0') * 2 + (instrucao_binaria[3] - '0'); 
    // Verifica o opcode e converter para instrução assembly correspondente
    switch(opcode) {
        case 0: // ADD
            fprintf(arquivo_asm, "ADD ");
            fprintf(arquivo_asm, "r%d, r%d, r%d\n", instrucao_binaria[4] - '0', instrucao_binaria[5] - '0', instrucao_binaria[6] - '0');
            break;
        case 1: // SUB
            fprintf(arquivo_asm, "SUB ");
            fprintf(arquivo_asm, "r%d, r%d, r%d\n", instrucao_binaria[4] - '0', instrucao_binaria[5] - '0', instrucao_binaria[6] - '0');
            break;
        case 2: // AND
            fprintf(arquivo_asm, "AND ");
            fprintf(arquivo_asm, "r%d, r%d, r%d\n", instrucao_binaria[4] - '0', instrucao_binaria[5] - '0', instrucao_binaria[6] - '0');
            break;
        case 3: // OR
            fprintf(arquivo_asm, "OR ");
            fprintf(arquivo_asm, "r%d, r%d, r%d\n", instrucao_binaria[4] - '0', instrucao_binaria[5] - '0', instrucao_binaria[6] - '0');
            break;
        case 4: // XOR
            fprintf(arquivo_asm, "XOR ");
            fprintf(arquivo_asm, "r%d, r%d, r%d\n", instrucao_binaria[4] - '0', instrucao_binaria[5] - '0', instrucao_binaria[6] - '0');
            break;
        default:
            fprintf(arquivo_asm, "Instrucao invalida\n");
            break;
    }
}

// Implemente as funções para executar o programa e as instruções
void executarInstrucao(Instrucao inst, BancoRegistradores *banco_registradores, PC *pc){
	int referencia = pc->endereco_atual; // Salva uma referencia caso algum procedimento não ocorra corretamente
	int endereco;
	switch(inst.tipo){
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
		 case I_TYPE:
			// Execução das instruções do tipo I
			switch (inst.opcode) {
				case 4: // addi
					banco_registradores->registradores[inst.rt] = banco_registradores->registradores[inst.rs] + inst.imm;
					memoria_dados[inst.rt] = banco_registradores->registradores[inst.rt];// Atualizar a memória de dados
					break;
				case 11: //Load word
				    endereco = banco_registradores->registradores[inst.rs] + inst.imm; // Endereço é a soma do registrador rs com o imediato
					banco_registradores->registradores[inst.rt] = memoria_dados[endereco]; // Carrega o dado para o registrador rt
				    break;
				case 15: //Store word
				    endereco = banco_registradores->registradores[inst.rs] + inst.imm; // Endereço é a soma do registrador rs com o imediato
					memoria_dados[endereco] = banco_registradores->registradores[inst.rt]; // Armazena o dado do registrador rt na memória de dados
				    break;
				case 8: // Brench on equal
				    if (banco_registradores->registradores[inst.rt] == banco_registradores->registradores[inst.rs]) {
						pc->endereco_atual = inst.imm; // Atualiza o PC para o endereço especificado pelo imediato
					}
					else{
						pc->endereco_atual = referencia;
					}
				    break;	
				}
		case J_TYPE:
			pc->endereco_atual = inst.addr; // Atualiza o PC para o endereço especificado na instrução
			if (pc->endereco_atual >= 0 && pc->endereco_atual < TAM_MEMORIA){ // Verifica se o endereço é válido
				printf("Desvio para o endereco: %d\n", pc->endereco_atual);
				pc->endereco_proximo = pc->endereco_atual; // Avança para a próxima instrução na sequência
			}
			else{
				pc->endereco_atual = referencia; // Define o próximo endereço como inválido e retorna
			}
			break;
	}
	pc->endereco_atual = pc->endereco_proximo; // Atualizar o endereço atual
	pc->endereco_proximo++; // Avançar para a próxima instrução na sequência
	printf("\nPC atual: %d\n", pc->endereco_atual);
	printf("PC proximo: %d\n", pc->endereco_proximo);
}


// FUNÇÕES DE INICIAÇÃO E ENTRADA

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
    // Inicializa a memória de dados preenchendo ela com zeros
    for (int i = 0; i < TAM_MEMORIA_DADOS; i++) {
        memoria_dados[i] = 0;
    }
}

void carregarMemoria() {
    FILE *arquivo_memoria = fopen("programaTestaInstrucoes.mem", "r"); //r = read / ponteiro para ler o arquivo
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

// Salvar instruções em .asm
void salvar_asm() {
    FILE *arquivo_asm = fopen("programa.asm", "w"); //w = write / ponteiro para escrever no arquivo
    if (arquivo_asm == NULL) {
        printf("Erro ao criar o arquivo\n");
        return;
    }
    // Iterar sobre cada instrução na memória e converter para assembly
    for (int i = 0; i < TAM_MEMORIA; i++) {
        if (strcmp(memoria_instrucao[i], "") != 0) { // Verifica se a instrução não está vazia
            fprintf(arquivo_asm, "Instrucao %d: ", i);
            fprintf(arquivo_asm, "%s\n", memoria_instrucao[i]);
        }
    }

    fclose(arquivo_asm);
    printf("Arquivo .asm salvo com sucesso!\n");
}

// Salvar configuração de memória em .mem 
void salvar_mem() {
    FILE *arquivo_mem = fopen("memoria.mem", "w"); //w = write / ponteiro para escrever no arquivo
    if (arquivo_mem == NULL) {
        printf("Erro ao criar o arquivo\n");
        return;
    }
    // Escrever o estado atual da memória no arquivo .mem
    for (int i = 0; i < TAM_MEMORIA; i++) {
        for (int j = 0; j < TAM_INSTRUCAO; j++) {
            fprintf(arquivo_mem, "%c ", memoria_instrucao[i][j]);
        }
        fprintf(arquivo_mem, "\n");
    }

    fclose(arquivo_mem);
    printf("Arquivo .mem salvo com sucesso!\n");
}



//FUNÇÕES DE IMPRESSÃO

void imprimirInstrucao(Instrucao inst) {
    switch (inst.tipo) {
        case R_TYPE:
            printf("Tipo: R_TYPE\t");
            printf("Opcode: %d\t", inst.opcode);
            printf("Rs (Origem): %d\t", inst.rs);
            printf("Rt (Alvo): %d\t", inst.rt);
            printf("Rd (Destino): %d\t", inst.rd);
            printf("Funct (Funcao(AND/SUB)): %d\t", inst.funct);
            break;
        case I_TYPE:
            printf("Tipo: I_TYPE\t");
            printf("Opcode: %d\t", inst.opcode);
            printf("Rs (Origem): %d\t", inst.rs);
            printf("Rt (Alvo): %d\t", inst.rt);
            printf("Imediato: %d\t", inst.imm);
            break;
        case J_TYPE:
            printf("Tipo: J_TYPE\t");
            printf("Opcode: %d\t", inst.opcode);
            printf("Addr: %d\t", inst.addr);
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







