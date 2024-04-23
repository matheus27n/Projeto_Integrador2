#include <string.h>
#include <stdlib.h>
#include "minimips.h"

int main(){
	BancoRegistradores banco_registradores;
    PC pc;
    inicializarBancoRegistradores(&banco_registradores);
    inicializarPC(&pc);
    carregarMemoria(); 
    inicializarMemoriaDados();
	int opcao = 1;
	while(opcao != 0){
		opcao = menu();
		switch(opcao){
			case 1:
                carregarMemoria();
                break;
            case 2:
				// Testa se a memória não é nula
				if(memoria_instrucao == NULL){
					carregarMemoria(); //Caso seja, carrega a memória automaticamente
				}
                imprimirMemoria(memoria_instrucao);
                break;
            case 3:
                imprimirRegistradores(&banco_registradores);
                break;
            case 4:
				// Testa se a memória não é nula
				if(memoria_instrucao == NULL){
					carregarMemoria(); //Caso seja, carrega a memória automaticamente
				}
                imprimirRegistradores(&banco_registradores);
                imprimirMemoriaDados();
                break;
            case 5:
                salvar_asm();
                break;
            case 6:
                salvar_mem();
                break;
            case 7:
                while (pc.endereco_atual < TAM_MEMORIA) {
                    executarInstrucao(codificarInstrucao(memoria_instrucao[pc.endereco_atual]), &banco_registradores, &pc);
                    printf("\n");
                }
                imprimirRegistradores(&banco_registradores);
                imprimirMemoriaDados();
                printf("\nPrograma executado com sucesso\n");
                break;
            case 8: 
                executarInstrucao(codificarInstrucao(memoria_instrucao[pc.endereco_atual]), &banco_registradores, &pc);
                break;
            case 9:
				break;
            case 0:
				printf("\nFinalizando o programa...\n\n");
				break;
            default:
                printf("Opcao invalida! Escolha uma opcao valida.\n");
		}
	}
}

