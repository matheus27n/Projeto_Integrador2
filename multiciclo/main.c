#include <string.h>
#include <stdlib.h>
#include "multiciclo.h"

int main(){
    BancoRegistradores banco_registradores;
    Controle unidadeControle;
    Instrucao instrucao;
    EstadoCiclo estado_ciclo;
    PC pc;
    struct nodo* backup = NULL; //Backup backup;
    inicializarBancoRegistradores(&banco_registradores);
    inicializarPC(&pc);

	int opcao = 1;
	while(opcao != 0){
		opcao = menu();
		switch(opcao){
			case 1:
                carregarMemoriaUnica(&unidadeControle);
                break;
            case 2:
                imprimirMemoriaUnica();
                break;
            case 3:
                imprimirRegistradores(&banco_registradores);
                break;
            case 4:
                imprimirMemoriaUnica();
                imprimirRegistradores(&banco_registradores);
				break;
            case 5:
                codificarInstrucao(memoria_instrucao[pc.endereco_atual]);
                break;
            case 6:
                salvar_asm();
                break;
            case 7:
                salvar_data();
                break;
            case 8:
                while(pc.endereco_atual<TAM_MEMORIA){
                    //executarClicoIntrucao();
                }
                imprimirMemoriaUnica();
                imprimirRegistradores(&banco_registradores);
                printf("programa finalizado\n");
                break;
            case 9:
                executarCicloInstrucao(&pc, &banco_registradores, &unidadeControle, instrucao, estado_ciclo);

                //backup = save_backup(&pc,memoria_dados,&banco_registradores);
                break;
            case 10: 
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

