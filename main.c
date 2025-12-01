#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_NOME 50
#define TAM_TEXTO 100
#define TAM_SUSPEITO 50
#define HASH_TAM 7   // tamanho pequeno para ver colisões

/* -------------------- Nível Novato: Árvore de salas -------------------- */

typedef struct Sala {
    char nome[TAM_NOME];
    struct Sala *esq;
    struct Sala *dir;
} Sala;

Sala *criarSala(const char *nome) {
    Sala *s = (Sala *)malloc(sizeof(Sala));
    if (!s) {
        printf("Erro ao alocar sala.\n");
        exit(1);
    }
    strcpy(s->nome, nome);
    s->esq = s->dir = NULL;
    return s;
}

void conectarSalas(Sala *pai, Sala *esq, Sala *dir) {
    if (pai) {
        pai->esq = esq;
        pai->dir = dir;
    }
}

/* -------------------- Nível Aventureiro: BST de pistas -------------------- */

typedef struct Pista {
    char texto[TAM_TEXTO];
    struct Pista *esq;
    struct Pista *dir;
} Pista;

Pista *criarPista(const char *texto) {
    Pista *p = (Pista *)malloc(sizeof(Pista));
    if (!p) {
        printf("Erro ao alocar pista.\n");
        exit(1);
    }
    strcpy(p->texto, texto);
    p->esq = p->dir = NULL;
    return p;
}

Pista *inserirBST(Pista *raiz, const char *texto) {
    if (raiz == NULL) {
        return criarPista(texto);
    }
    int cmp = strcmp(texto, raiz->texto);
    if (cmp < 0) {
        raiz->esq = inserirBST(raiz->esq, texto);
    } else if (cmp > 0) {
        raiz->dir = inserirBST(raiz->dir, texto);
    }
    // se igual, não insere duplicado (poderia tratar diferente se quiser)
    return raiz;
}

void emOrdem(Pista *raiz) {
    if (raiz == NULL) return;
    emOrdem(raiz->esq);
    printf("- %s\n", raiz->texto);
    emOrdem(raiz->dir);
}

/* -------------------- Nível Mestre: Hash de suspeitos -------------------- */

typedef struct NodoPistaAssoc {
    char texto[TAM_TEXTO];
    struct NodoPistaAssoc *prox;
} NodoPistaAssoc;

typedef struct Suspeito {
    char nome[TAM_SUSPEITO];
    int contPistas;
    NodoPistaAssoc *listaPistas;
    struct Suspeito *prox; // para lista encadeada na hash
} Suspeito;

typedef struct {
    Suspeito *tabela[HASH_TAM];
} HashSuspeitos;

void inicializarHash(HashSuspeitos *h) {
    for (int i = 0; i < HASH_TAM; i++) {
        h->tabela[i] = NULL;
    }
}

int funcaoHash(const char *chave) {
    int soma = 0;
    for (int i = 0; chave[i] != '\0'; i++) {
        soma += (unsigned char)chave[i];
    }
    return soma % HASH_TAM;
}

Suspeito *buscarSuspeito(HashSuspeitos *h, const char *nome) {
    int idx = funcaoHash(nome);
    Suspeito *atual = h->tabela[idx];
    while (atual) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

Suspeito *criarSuspeito(const char *nome) {
    Suspeito *s = (Suspeito *)malloc(sizeof(Suspeito));
    if (!s) {
        printf("Erro ao alocar suspeito.\n");
        exit(1);
    }
    strcpy(s->nome, nome);
    s->contPistas = 0;
    s->listaPistas = NULL;
    s->prox = NULL;
    return s;
}

void inserirPistaNaLista(Suspeito *s, const char *texto) {
    NodoPistaAssoc *n = (NodoPistaAssoc *)malloc(sizeof(NodoPistaAssoc));
    if (!n) {
        printf("Erro ao alocar nodo de pista.\n");
        exit(1);
    }
    strcpy(n->texto, texto);
    n->prox = s->listaPistas;
    s->listaPistas = n;
    s->contPistas++;
}

void inserirHash(HashSuspeitos *h, const char *nomeSuspeito, const char *textoPista) {
    int idx = funcaoHash(nomeSuspeito);
    Suspeito *s = buscarSuspeito(h, nomeSuspeito);
    if (!s) {
        s = criarSuspeito(nomeSuspeito);
        s->prox = h->tabela[idx];
        h->tabela[idx] = s;
    }
    inserirPistaNaLista(s, textoPista);
}

void listarAssociacoes(HashSuspeitos *h) {
    printf("\n=== Suspeitos e Pistas Associadas (Hash) ===\n");
    for (int i = 0; i < HASH_TAM; i++) {
        Suspeito *s = h->tabela[i];
        while (s) {
            printf("Suspeito: %s (pistas: %d)\n", s->nome, s->contPistas);
            NodoPistaAssoc *p = s->listaPistas;
            while (p) {
                printf("   - %s\n", p->texto);
                p = p->prox;
            }
            s = s->prox;
        }
    }
}

Suspeito *suspeitoMaisProvavel(HashSuspeitos *h) {
    Suspeito *melhor = NULL;
    for (int i = 0; i < HASH_TAM; i++) {
        Suspeito *s = h->tabela[i];
        while (s) {
            if (melhor == NULL || s->contPistas > melhor->contPistas) {
                melhor = s;
            }
            s = s->prox;
        }
    }
    return melhor;
}

/* -------------------- Ligações salas → pistas → suspeitos -------------------- */

void adicionarPistasDaSala(Sala *sala, Pista **raizPistas, HashSuspeitos *hash) {
    if (strcmp(sala->nome, "Biblioteca") == 0) {
        *raizPistas = inserirBST(*raizPistas, "Livro rasgado com nome do mordomo");
        inserirHash(hash, "Mordomo", "Livro rasgado com nome do mordomo");
    } else if (strcmp(sala->nome, "Cozinha") == 0) {
        *raizPistas = inserirBST(*raizPistas, "Faca suja de sangue");
        inserirHash(hash, "Chef", "Faca suja de sangue");
    } else if (strcmp(sala->nome, "Sotao") == 0) {
        *raizPistas = inserirBST(*raizPistas, "Foto antiga do dono com o jardineiro");
        inserirHash(hash, "Jardineiro", "Foto antiga do dono com o jardineiro");
    } else if (strcmp(sala->nome, "Sala Secreta") == 0) {
        *raizPistas = inserirBST(*raizPistas, "Contrato de heranca alterado");
        inserirHash(hash, "Heredeiro", "Contrato de heranca alterado");
    }
}

/* -------------------- Exploração da mansão -------------------- */

void explorarSalas(Sala *raiz, Pista **raizPistas, HashSuspeitos *hash) {
    Sala *atual = raiz;
    char opcao;
    if (!atual) return;

    while (1) {
        printf("\nVoce esta na sala: %s\n", atual->nome);
        adicionarPistasDaSala(atual, raizPistas, hash);

        printf("Mover para (e) esquerda, (d) direita, (p) ver pistas, (h) ver suspeitos, (s) sair: ");
        scanf(" %c", &opcao);

        if (opcao == 's') {
            printf("Encerrando exploracao da mansao...\n");
            break;
        } else if (opcao == 'e') {
            if (atual->esq) {
                atual = atual->esq;
            } else {
                printf("Nao ha sala à esquerda.\n");
            }
        } else if (opcao == 'd') {
            if (atual->dir) {
                atual = atual->dir;
            } else {
                printf("Nao ha sala à direita.\n");
            }
        } else if (opcao == 'p') {
            printf("\n=== Pistas em ordem alfabetica (BST) ===\n");
            if (*raizPistas == NULL) {
                printf("Nenhuma pista coletada ainda.\n");
            } else {
                emOrdem(*raizPistas);
            }
        } else if (opcao == 'h') {
            listarAssociacoes(hash);
            Suspeito *suspeito = suspeitoMaisProvavel(hash);
            if (suspeito) {
                printf("\nSuspeito mais provavel ate agora: %s (pistas: %d)\n",
                       suspeito->nome, suspeito->contPistas);
            } else {
                printf("\nNenhum suspeito registrado ainda.\n");
            }
        } else {
            printf("Opcao invalida.\n");
        }
    }
}

/* -------------------- Liberação de memória (opcional, mas bom hábito) -------------------- */

void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esq);
    liberarSalas(raiz->dir);
    free(raiz);
}

void liberarPistas(Pista *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz);
}

void liberarHash(HashSuspeitos *h) {
    for (int i = 0; i < HASH_TAM; i++) {
        Suspeito *s = h->tabela[i];
        while (s) {
            Suspeito *tmpS = s;
            s = s->prox;

            NodoPistaAssoc *p = tmpS->listaPistas;
            while (p) {
                NodoPistaAssoc *tmpP = p;
                p = p->prox;
                free(tmpP);
            }
            free(tmpS);
        }
    }
}

/* -------------------- main: integra os três níveis -------------------- */

int main() {
    // Monta árvore fixa de salas (Novato)
    Sala *hall = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *cozinha = criarSala("Cozinha");
    Sala *sotao = criarSala("Sotao");
    Sala *salaSecreta = criarSala("Sala Secreta");

    // Estrutura da mansão (exemplo simples):
    //           Hall
    //         /      \
    //   Biblioteca   Cozinha
    //      /            \
    //   Sotao        Sala Secreta
    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, sotao, NULL);
    conectarSalas(cozinha, NULL, salaSecreta);

    // BST de pistas (Aventureiro)
    Pista *raizPistas = NULL;

    // Hash de suspeitos (Mestre)
    HashSuspeitos hash;
    inicializarHash(&hash);

    printf("=== Desafio Detective Quest ===\n");
    explorarSalas(hall, &raizPistas, &hash);

    // Liberação de memória
    liberarSalas(hall);
    liberarPistas(raizPistas);
    liberarHash(&hash);

    return 0;
}
