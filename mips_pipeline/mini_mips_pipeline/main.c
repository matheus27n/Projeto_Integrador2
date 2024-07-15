#include <string.h>
#include <stdlib.h>
#include "pipeline.h"

int main(){
    BancoRegistradores banco_registradores;
    PC pc;
    //Backup backup;
    struct nodo* backup = NULL;
    //Instrucao inst;
    inicializarBancoRegistradores(&banco_registradores);
    inicializarPC(&pc);
    inicializarMemoriaDados();

	int opcao = 1;
	while(opcao != 0){
		opcao = menu();
		switch(opcao){
			case 1:
                carregarMemoria();
                break;
            case 2:
                carregarMemoriaDados();
                break;
            case 3:
				// Testa se a memória não é nula
				if(memoria_instrucao == NULL){
					carregarMemoria(); //Caso seja, carrega a memória automaticamente
				}
                imprimirMemoria(memoria_instrucao);
                break;
            case 4:
				imprimirMemoriaDados();
				break;
            case 5:
                imprimirRegistradores(&banco_registradores);
                break;
            case 6:
				// Testa se a memória não é nula
				if(memoria_instrucao == NULL){
					carregarMemoria(); //Caso seja, carrega a memória automaticamente
				}
                imprimirRegistradores(&banco_registradores);
                imprimirMemoriaDados();
                break;
            case 7:
                salvar_asm();
                break;
            case 8:
                salvar_data();
                break;
            case 9:
                //executar todo o programa
                executarTodoPipeline();
                break;
            case 10: 
                //executar uma instrução por vez
                executarPipeline();
                break;
            case 11:
                undo(backup,&pc,memoria_dados,&banco_registradores);
                break;
            case 0:
				printf("\nFinalizando o programa...\n\n");
				break;
            default:
                printf("Opcao invalida! Escolha uma opcao valida.\n");
		}
	}
	free(backup); // Libere a memória alocada para o backup
        return 0;
}

