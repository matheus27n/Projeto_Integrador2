#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "pipeline.h"

// Definição do menu
char *choices[] = {
    "1. Carregar memoria",
    "2. Carregar memoria de dados",
    "3. Imprimir memoria de instruções",
    "4. Imprimir memoria de dados",
    "5. Imprimir registradores",
    "6. Imprimir todo o simulador",
    "7. Salvar .asm",
    "8. Salvar .data",
    "9. Executa Programa (run)",
    "10. Executa uma instrucao (Step pipeline)",
    "11. Volta uma instrucao (Back)",
    "0. Sair",
};

int n_choices = sizeof(choices) / sizeof(char *);

void print_menu(WINDOW *menu_win, int highlight);
void execute_option(int choice, BancoRegistradores *banco_registradores, PC *pc, struct nodo **backup);

int main() {
    BancoRegistradores banco_registradores;
    PC pc;
    struct nodo* backup = NULL;

    inicializarBancoRegistradores(&banco_registradores);
    inicializarPC(&pc);
    inicializarMemoriaDados();

    initscr();
    clear();
    noecho();
    cbreak(); // Line buffering disabled. Pass on everything

    int startx = 0;
    int starty = 0;
    int width = 50;
    int height = 20;

    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);
    int choice = 0;
    int highlight = 1;

    while (1) {
        print_menu(menu_win, highlight);
        int c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 10: // Enter key
                choice = highlight;
                execute_option(choice, &banco_registradores, &pc, &backup);
                if (choice == 12) { // Sair
                    goto end;
                }
                break;
            default:
                refresh();
                break;
        }
    }

end:
    clrtoeol();
    refresh();
    endwin();
    free(backup);
    return 0;
}

void print_menu(WINDOW *menu_win, int highlight) {
    int x = 2, y = 2;
    box(menu_win, 0, 0);

    // Adiciona o título no topo do menu
    mvwprintw(menu_win, 1, (50 - strlen("MINI MIPS PIPELINE - UNIPAMPA")) / 2, "MINI MIPS PIPELINE - UNIPAMPA");

    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y + 2, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y + 2, x, "%s", choices[i]);
        }
        ++y;
    }
    wrefresh(menu_win);
}

void execute_option(int choice, BancoRegistradores *banco_registradores, PC *pc, struct nodo **backup) {
    endwin(); // End ncurses mode temporarily to allow normal terminal output

    switch (choice) {
        case 1:
            carregarMemoria();
            break;
        case 2:
            carregarMemoriaDados();
            break;
        case 3:
            imprimirMemoria(memoria_instrucao);
            break;
        case 4:
            imprimirMemoriaDados();
            break;
        case 5:
            imprimirRegistradores(banco_registradores);
            break;
        case 6:
            imprimirRegistradores(banco_registradores);
            imprimirMemoriaDados();
            break;
        case 7:
            salvar_asm();
            break;
        case 8:
            salvar_data();
            break;
        case 9:
            executarTodoPipeline();
            break;
        case 10:
            executarPipeline(pc, banco_registradores, NULL, NULL, NULL, NULL, NULL);
            break;
        case 11:
            undo(*backup, pc, memoria_dados, banco_registradores);
            break;
        case 12:
            printf("\nFinalizando o programa...\n\n");
            exit(0); // Terminate the program
        default:
            printf("Opcao invalida! Escolha uma opcao valida.\n");
    }

    // Pausa para o usuário ver a saída
    printf("Pressione qualquer tecla para voltar ao menu...");
    getchar();
    getchar(); // This ensures we capture the newline character from previous input

    initscr(); // Reinitialize ncurses mode
    clear();
    noecho();
    cbreak();
    refresh();
}
